#pragma once

#include "memory.h"
#include "opres.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
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

    explicit storage(size_t dynamic_size=0)requires(dim==DYNAMIC)
    {
        const std::size_t allocsz=std::max(Mincpct, dynamic_size);
        this->_data=(T*)alloc(allocsz*sizeof(T));
        if(!_data)
            throw std::bad_alloc();
        this->_dynmsz=dynamic_size;
        this->_capacity=allocsz;
    }
        /*COPY*/

    //noop land. this should shut up the compiler
    storage()noexcept requires(dim!=DYNAMIC){}
    
    //use copy(). be explicit.
    storage(const storage&)=delete;
    
    /*OPERATORS*/

        /*OPERATOR=*/

    storage& operator=(const storage&)=delete;
    
    /*DATA*/

private:
    inline void destroy(T* begin, T* end)
        noexcept(std::is_nothrow_destructible_v<T>)
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
            while(end!=begin) (--end)->~T();
    }
public:
    inline opres rsvcpct(size_t newcpct) 
        noexcept(std::is_nothrow_destructible_v<T>&&
            (std::is_nothrow_move_constructible_v<T> || 
                (!std::is_nothrow_move_constructible_v<T>&&
                     std::is_nothrow_copy_constructible_v<T>)))
            requires(dim==DYNAMIC)
    {
        if(newcpct<=this->_capacity)   //noalloc
            return opres::SUCCESS;

        T* newaddr=(T*)alloc(sizeof(T)*newcpct);
        if(!newaddr) //alloc fault
            return opres::OOM;
        
        static_assert(std::is_move_constructible_v<T> || 
            std::is_copy_constructible_v<T>, "tf you on bro");
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
            
        destroy(this->begin(), this->end());

        free(this->_data);
        this->_data=newaddr;
        this->_capacity=newcpct;

        return opres::SUCCESS;
    }
    
    inline opres resize(size_t newsize)
        noexcept(noexcept(rsvcpct(std::declval<size_t>())))
            requires(dim==DYNAMIC)
    {
        if(newsize==this->size())   //noop
            return opres::SUCCESS;
        if(newsize<this->size())    //shrink
        {
            destroy(this->begin() + newsize, this->end());
            this->_dynmsz=newsize;
            return opres::SUCCESS;
        }
        auto res=rsvcpct(newsize); //check _capacity
        if(res==opres::SUCCESS)
            this->_dynmsz=newsize;
        return res;
    }

    template<typename U=T, 
        size_t OthrMincpt=Mincpct, 
            memalloc_t OthrAlloc=alloc, memfree_t OthrFree=free>
    inline storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree>
        copy(size_t n_elements)const //must satisfy <= this->size()
            noexcept(std::is_nothrow_constructible_v<U, const T&>&&
                std::is_nothrow_copy_constructible_v<T>)
            requires(dim==DYNAMIC&&std::is_copy_constructible_v<T>&&
                std::is_constructible_v<U, const T&>)
    {
        //TODO
    }

    /*DESTRUCTOR*/

    ~storage()
        noexcept(noexcept(destroy(std::declval<T*>(), std::declval<T*>())))
    {
        destroy(this->begin(), this->end());
        free(this->_data);
    }
};
}; //namespace aico















