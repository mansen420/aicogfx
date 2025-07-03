#pragma once

#include "memory.h"
#include "opres.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include <new>
#include <type_traits>

namespace aico
{

inline constexpr size_t DYNAMIC = 0;

typedef void*(*memalloc_t)(size_t);
typedef void(*memfree_t)(void*);

template <typename T, size_t dim=DYNAMIC, bool inlined=(dim!=DYNAMIC), 
    size_t Mincpct=8,
        memalloc_t alloc=[](size_t sz){return sys::memalloc(sz);}, 
            memfree_t free=&sys::memfree>
    requires (!(inlined && dim == DYNAMIC)&&Mincpct>0)
       class storage
{
public: //XXX testing
    struct nothing_t
    {
        template <typename...types>
            nothing_t(types...){}
        
        template <typename...types>
            nothing_t& operator=(types...){return *this;}
        
        template <typename U>
            operator U()const{return U(0);}
    };

    template<typename, size_t sz, bool inl, size_t Mcpt, memalloc_t, memfree_t>
        requires (!(inl && sz == DYNAMIC)&&Mcpt>0)
            friend class storage;

    [[no_unique_address]] std::conditional_t<dim == DYNAMIC, size_t, nothing_t> 
        _dynmsz;         //number of elements
    [[no_unique_address]] std::conditional_t<dim == DYNAMIC, size_t, nothing_t> 
        _capacity;       //maximum capacity of elements. always dynmsz<=capacity.
    alignas(inlined?alignof(T):alignof(T*)) 
        std::conditional_t<inlined, char[sizeof(T)*dim], T*> _data;
public:
    /*SIZE*/

    /// @return Number of elements in the vector evaluated at compile time.
    inline constexpr size_t size()const requires(dim != DYNAMIC){return dim;}
    /// @return Number of elements in the vector fetched at runtime. 
    inline size_t size()const requires(dim == DYNAMIC){return _dynmsz;}

    /*INDEXING*/
    
        /*STATIC*/

    template<size_t idx> 
        inline T& at()
            requires(dim!=DYNAMIC)
    {
        static_assert(idx<dim, "out of bounds");
        return ((T*)this->_data)[idx];
    }
    template<size_t idx>
        inline const T& at()
            const requires(dim!=DYNAMIC)
    {
        static_assert(idx<dim, "out of bounds");
        return ((T*)this->_data)[idx];
    }

        /*DYNAMIC*/

    inline T& at(size_t idx)noexcept
    {
        assert(idx<this->size() && "out of bounds");
        return ((T*)this->_data)[idx];
    }
    inline const T& at(size_t idx) const noexcept
    {
        assert(idx<this->size() && "out of bounds");
        return ((T*)this->_data)[idx];
    }

        /*SUBSCRIPT*/

    inline const T& operator[](size_t idx) const noexcept{return at(idx);}
    inline       T& operator[](size_t idx)       noexcept{return at(idx);}
        
    /*ITERATOR*/

    inline const T* begin()const noexcept{return (T*)_data;}
    inline       T* begin()      noexcept{return (T*)_data;}
    //T* arithmetic is triggered via begin()
    inline const T* end()const noexcept{return this->begin()+this->size();}
    inline       T* end()      noexcept{return this->begin()+this->size();}
 

    /*CONSTRUCTOR*/
    //TODO

    void _initcpct(size_t logical_size)
    {
        const std::size_t allocsz=std::max(Mincpct, logical_size);
        this->_data=(T*)alloc(allocsz*sizeof(T));
        if(!_data)
            throw std::bad_alloc();
        this->_dynmsz=logical_size;
        this->_capacity=allocsz;
    }
    //user is responsible for initialization
    explicit storage(size_t dynamic_size=0)
        requires(dim==DYNAMIC&&!std::is_default_constructible_v<T>)
    {
        _initcpct(dynamic_size);
    }
    //default construction
    explicit storage(size_t dynamic_size=0)
        requires(dim==DYNAMIC&&requires{T();})
    {
        _initcpct(dynamic_size);
        std::uninitialized_default_construct_n(this->_data, this->_dynmsz);
    }
    //copy construction
    explicit storage(size_t dynamic_size, const T& fillval)
        requires(dim==DYNAMIC&&requires{T(std::declval<const T&>());})
    {
        _initcpct(dynamic_size);
        std::uninitialized_fill_n(this->begin(), this->size(), fillval);
    }
    //custom construction
    template<typename...Args>
        explicit storage(size_t dynamic_size, Args&&...args)
            requires(dim==DYNAMIC&&requires{T(std::declval<Args>()...);})
    {
        _initcpct(dynamic_size);
        for(size_t i=0; i < dynamic_size; ++i)
            std::construct_at(this->_data+i, std::forward<Args>(args)...);
    }
    
    //take ownership of preinitialized data
    //Precondition: `data` must point to `dynamic_size` *fully constructed* Ts
    //`dynamic_size` must be >= 0, and data must point to at least Mincpct objects
    explicit storage(T* data, size_t dynamic_size)noexcept requires(dim==DYNAMIC)
    {
        assert(data!=nullptr);
        this->_data=data;
        this->_dynmsz=dynamic_size;
        this->_capacity=std::max(Mincpct, dynamic_size);
    }

    //noop land. this should shut up the compiler
    storage()noexcept requires(dim!=DYNAMIC){}

        /*COPY*/

    //use copy(). be explicit.
    storage(const storage&)=delete;
    
        /*MOVE*/
    
    storage(storage&&other)noexcept requires(dim==DYNAMIC)
    {
        this->_capacity=other._capacity;
        this->_dynmsz=other._dynmsz;
        this->_data=other._data;
        
        other._data=nullptr;
        other._capacity=0;
        other._dynmsz=0;
    }

    /*OPERATORS*/

        /*OPERATOR=*/

    storage& operator=(const storage&)=delete;
    
    /*DATA*/
        
    inline void _destroy(T* begin, T* end)
        noexcept(std::is_nothrow_destructible_v<T>)
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
            while(end!=begin) (--end)->~T();
    }
        
        /*RESERVE*/

    inline opres rsvcpct(size_t newcpct) 
    noexcept(std::is_nothrow_destructible_v<T>&&
                (std::is_nothrow_move_constructible_v<T> || 
                    (!std::is_nothrow_move_constructible_v<T>&&
                     std::is_nothrow_copy_constructible_v<T>)))
    requires(dim==DYNAMIC&&std::is_destructible_v<T>&&
                (requires{T(std::declval<T&&>());}||
                    (!requires{T(std::declval<T&&>());}&&
                     requires{T(std::declval<const T&>());})))
    {
        if(newcpct<=this->_capacity)   //noalloc
            return opres::SUCCESS;

        T* newaddr=(T*)alloc(sizeof(T)*newcpct);
        if(!newaddr) //alloc fault
            return opres::MEM_ERR;
        
        try
        {
            if constexpr (std::is_trivially_copyable_v<T>)
                memcpy(newaddr, this->_data, this->size()*sizeof(T));
            else
                if constexpr(std::is_move_constructible_v<T>)
                    std::uninitialized_move(this->begin(), this->end(), newaddr);
                else
                    std::uninitialized_copy(this->begin(), this->end(), newaddr);
        }
        catch(...)
        {
            free(newaddr);
            throw;
        }
            
        _destroy(this->begin(), this->end());

        free(this->_data);
        this->_data=newaddr;
        this->_capacity=newcpct;

        return opres::SUCCESS;
    }
        
        /*RESIZE*/
        
    inline opres _resize_noinit(size_t newsize)
    noexcept(noexcept(rsvcpct(std::declval<size_t>())))
    requires(dim==DYNAMIC)
    {
        if(newsize==this->size())   //noop
            return opres::SUCCESS;
        if(newsize<this->size())    //shrink
        {
            _destroy(this->begin() + newsize, this->end());
            this->_dynmsz=newsize;
            return opres::SUCCESS;
        }
        auto res=rsvcpct(newsize); //check _capacity
        if(res==opres::SUCCESS)
            this->_dynmsz=newsize;
        return res;
    }
    //performs no initialization
    inline opres resize(size_t newsize)
        noexcept(noexcept(_resize_noinit(std::declval<size_t>())))
        requires(dim==DYNAMIC&&!requires{T();}&&
                 !requires{T();})
    {
        auto res=this->_resize_noinit(newsize);
        return res;
    }
    inline opres resize(size_t newsize)
        noexcept(noexcept(_resize_noinit(std::declval<size_t>()))&&
                          std::is_nothrow_default_constructible_v<T>)
        requires(dim==DYNAMIC&&requires{T();})
    {
        size_t oldsize=this->size();
        auto res=this->_resize_noinit(newsize);
        if(newsize>oldsize&&res==opres::SUCCESS)
            std::uninitialized_default_construct_n(this->end(), newsize-_dynmsz);
        return res;
    }
    inline opres resize(size_t newsize, const T& fillval)
        noexcept(noexcept(_resize_noinit(std::declval<size_t>()))&&
                          std::is_nothrow_constructible_v<T, const T&>)
        requires(dim==DYNAMIC&&requires{T(std::declval<const T&>());})
    {
        size_t oldsize=this->size();
        auto res=this->_resize_noinit(newsize);
        if(newsize>oldsize&&res==opres::SUCCESS)
            std::uninitialized_fill_n(this->end(), newsize-_dynmsz, fillval);
        return res;
    }
    template<typename...Args>
    inline opres resize(size_t newsize, Args&&...args)
        noexcept(noexcept(_resize_noinit(std::declval<size_t>()))&&
                          std::is_nothrow_constructible_v<T, Args...>)
        requires(dim==DYNAMIC&&requires{T(std::declval<Args>()...);})
    {
        size_t oldsize=this->size();
        auto res=this->_resize_noinit(newsize);
        if(newsize>oldsize&&res==opres::SUCCESS)
            for(size_t i=oldsize; i<newsize; ++i)
                std::construct_at(this->begin()+i, 
                    std::forward<Args>(args)...);
        return res;
    }

        /*COPY*/

    template<typename U=T, 
        size_t OthrMincpt=Mincpct, 
        memalloc_t OthrAlloc=alloc, memfree_t OthrFree=free>
    inline storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree>
    copy(size_t n_elements, size_t fromidx=0, opres*res=nullptr)const  
        noexcept(std::is_nothrow_constructible_v<U, const T&>)
        requires(dim==DYNAMIC&&requires{U(std::declval<const T&>());})
    {
        assert(n_elements+fromidx<=this->size());
        if(n_elements+fromidx>this->size())
        {
            if(res)
                *res=opres::BOUNDS_ERR;
            return storage{};
        }
        //allocate raw memory
        U* resdata=(U*)OthrAlloc(std::max(n_elements, OthrMincpt)*sizeof(U));
        //initialize it
        std::uninitialized_copy_n(this->begin()+fromidx, n_elements, resdata);
        //pass it
        storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree> 
            result((U*)resdata, (size_t)n_elements);

        if(res)
            *res=opres::SUCCESS;

        return result; //moved due to RVO
    }

    template<typename U=T, 
        size_t OthrMincpt=Mincpct, 
        memalloc_t OthrAlloc=alloc, memfree_t OthrFree=free>
    inline storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree>
    copy()const  
        noexcept(std::is_nothrow_constructible_v<U, const T&>)
        requires(dim==DYNAMIC&&requires{U(std::declval<const T&>());})
    {
        return this->copy<U, OthrMincpt, OthrAlloc, OthrFree>(this->size());
    }
    
    //initialize: whether the destination memory should be initialized 
    //(constructed) by this function 
    template<typename U=T, 
        size_t OthrMincpt=Mincpct, 
        memalloc_t OthrAlloc=alloc, memfree_t OthrFree=free>
    inline opres copy(
            size_t n_elements,
            storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree>&dst,
            size_t dst_startidx=0, 
            size_t src_startidx=0,
            bool initialize=false
            )
        noexcept(std::is_nothrow_constructible_v<U, const T&>)
        requires(dim==DYNAMIC&&requires{U(std::declval<const T&>());})
    {
        assert(n_elements+src_startidx<=this->size());
        if(n_elements+src_startidx>this->size())
            return opres::BOUNDS_ERR;
        assert(n_elements+dst_startidx<=dst.size());
        if(n_elements+dst_startidx>dst.size())
            return opres::BOUNDS_ERR;

        if constexpr(std::is_trivially_copyable_v<T>&&(std::is_same_v<T, U>))
            memcpy(dst.begin()+dst_startidx, this->begin()+src_startidx, 
                n_elements*sizeof(U));
        else
            if(initialize)
                std::uninitialized_copy_n(this->begin()+src_startidx, n_elements, 
                    dst.begin()+dst_startidx);
            else
                std::copy_n(this->begin()+src_startidx, n_elements, 
                    dst.begin()+dst_startidx);
        return opres::SUCCESS;
    }
    
            /*MOVE*/
    
    template<typename U=T, 
        size_t OthrMincpt=Mincpct, 
        memalloc_t OthrAlloc=alloc, memfree_t OthrFree=free>
    inline storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree>
    move(size_t n_elements, size_t fromidx=0, opres*res=nullptr)
        noexcept(std::is_nothrow_constructible_v<U, T&&>)
        requires(dim==DYNAMIC&&requires{U(std::declval<T&&>());})
    {
        assert(n_elements+fromidx<=this->size());
        if(n_elements+fromidx>this->size())
        {
            if(res)
                *res=opres::BOUNDS_ERR;
            return storage{};
        }
        //allocate raw memory
        U* resdata=(U*)OthrAlloc(std::max(n_elements, OthrMincpt)*sizeof(U));
        //initialize it
        std::uninitialized_move_n(this->begin()+fromidx, n_elements, resdata);
        //pass it
        storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree> 
            result((U*)resdata, (size_t)n_elements);
        
        if(res)
            *res=opres::SUCCESS;
        return result; //moved due to RVO
    }

    template<typename U=T,
        size_t OthrMincpt=Mincpct, 
        memalloc_t OthrAlloc=alloc, memfree_t OthrFree=free>
    inline storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree>
    move()
        noexcept(std::is_nothrow_constructible_v<U, T&&>)
        requires(dim==DYNAMIC&&requires{U(std::declval<T&&>());})
    {
        return this->move<U, OthrMincpt, OthrAlloc, OthrFree>(this->size());
    }

    //initialize: whether the destination memory should be initialized 
    //(constructed) by this function
    template<typename U=T, 
        size_t OthrMincpt=Mincpct, 
        memalloc_t OthrAlloc=alloc, memfree_t OthrFree=free>
    inline opres move(
                size_t n_elements,
                storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree>&dst,
                size_t dst_startidx=0,
                size_t src_startidx=0,
                bool initialize=false
            )
    noexcept(std::is_nothrow_constructible_v<U, T&&>)
    requires(dim==DYNAMIC&&requires{U(std::declval<T&&>());})
    {
        assert(n_elements+src_startidx<=this->size());
        if(n_elements+src_startidx>this->size())
            return opres::BOUNDS_ERR;
        assert(n_elements+dst_startidx<=dst.size());
        if(n_elements+dst_startidx>dst.size())
            return opres::BOUNDS_ERR;
        
        const auto start=this->begin()+src_startidx;
        if(initialize)
            std::uninitialized_move(start, start+n_elements, dst.begin()+dst_startidx);
        else
            std::move(start, start+n_elements, dst.begin()+dst_startidx);

        return opres::SUCCESS;
    }


    /*DESTRUCTOR*/
    
    ~storage()
        noexcept(std::is_nothrow_destructible_v<T>)
        requires(dim==DYNAMIC)
    {
        if(!this->_data)//invalid
            return;
        _destroy(this->begin(), this->end());
        free(this->_data);
    }
    ~storage()noexcept requires(dim!=DYNAMIC)=default;
};
}; //namespace aico
