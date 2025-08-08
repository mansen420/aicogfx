#pragma once

#include "malc.h"
#include "memory.h"
#include "opres.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <type_traits>

namespace aico
{

    inline void copy_bits(const uint8_t* src_bits,
                   size_t from_bitidx,
                   size_t n_bits,
                   uint8_t* dst_bits)
        {
            if(n_bits==0) return;

            const size_t src_byte_start = from_bitidx / 8;
            const size_t src_bit_offset = from_bitidx % 8;

            const size_t dst_bytes = (n_bits + 7) / 8;

            if (src_bit_offset == 0) 
            {
                // Easy case: byte aligned
                if (n_bits >= 8) 
                {
                    // Copy whole bytes
                    size_t full_bytes = n_bits / 8;
                    std::memcpy(dst_bits, src_bits + src_byte_start, 
                        full_bytes);

                    // Handle tail bits
                    size_t tail_bits = n_bits % 8;
                    if (tail_bits) {
                        uint8_t mask = uint8_t((1u << tail_bits) - 1);
                        dst_bits[full_bytes] = (src_bits[src_byte_start + full_bytes] &
                            mask);
                    }
                } 
                else 
                {
                    // Fits entirely within one byte
                    uint8_t mask = uint8_t((1u << n_bits) - 1);
                    dst_bits[0] = (src_bits[src_byte_start] & mask);
                }
            } 
            else 
            {
                // Unaligned: need to shift bits across byte boundaries
                size_t bitpos = 0;
                size_t bits_remaining = n_bits;

                // First partial byte
                uint8_t first_src = src_bits[src_byte_start] >> src_bit_offset;
                uint8_t next_src = 0;
                if (src_bit_offset + bits_remaining > 8 && src_bits[src_byte_start + 1])
                    next_src = src_bits[src_byte_start + 1] << (8 - src_bit_offset);

                uint8_t combined = first_src | next_src;
                size_t bits_in_first = std::min<size_t>(8, bits_remaining);
                uint8_t mask = uint8_t((1u << bits_in_first) - 1);
                dst_bits[0] = combined & mask;

                bitpos += bits_in_first;
                bits_remaining -= bits_in_first;

                // Middle full bytes
                size_t dst_index = 1;
                while (bits_remaining >= 8) 
                {
                    uint8_t lo = src_bits[src_byte_start + (bitpos / 8)] >>
                        src_bit_offset;
                    uint8_t hi = src_bits[src_byte_start + (bitpos / 8) + 1] <<
                        (8 - src_bit_offset);
                    dst_bits[dst_index++] = lo | hi;
                    bitpos += 8;
                    bits_remaining -= 8;
                }

                // Tail partial byte
                if (bits_remaining > 0) 
                {
                    uint8_t lo = src_bits[src_byte_start + (bitpos / 8)] >> 
                        src_bit_offset;
                    uint8_t hi = 0;
                    if (src_bit_offset + bits_remaining > 8)
                        hi = src_bits[src_byte_start + (bitpos / 8) + 1] <<
                            (8 - src_bit_offset);
                    uint8_t tail_combined = lo | hi;
                    uint8_t tail_mask = uint8_t((1u << bits_remaining) - 1);
                    dst_bits[dst_index] = tail_combined & tail_mask;
                }
            }
        }
inline constexpr size_t DYNAMIC = 0;

typedef void*(*memalloc_t)(size_t);
typedef void(*memfree_t)(void*);

inline void* alloc_bind(size_t sz){return sys::malc(sz);}

template <typename T, size_t dim=DYNAMIC, bool inlined=(dim!=DYNAMIC), 
    size_t Mincpct=8, memalloc_t Alloc=&alloc_bind, memfree_t Free=&sys::rel>
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
        
        template<typename U>
            bool operator ==(const U&)const{return false;}

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

    static constexpr bool Alivebit_Cond=!(std::is_trivial_v<T>&&
        std::is_standard_layout_v<T>);
    [[no_unique_address]] std::conditional_t<dim == DYNAMIC&&Alivebit_Cond, 
        uint8_t*, 
        nothing_t> _alivebits;

    alignas(inlined?alignof(T):alignof(T*)) 
        std::conditional_t<inlined, char[sizeof(T)*dim], T*> _data;
public:
    /*BITS*/
    
    inline constexpr size_t _n_bytes(size_t n_bits)const noexcept
    {
        return n_bits/8+((n_bits%8)>0); //+1 for remainder
    }
    inline void _setbit(size_t idx)noexcept requires(Alivebit_Cond)
    {
        if(_alivebits) _alivebits[idx/8]|=(1<<idx%8);
    }
    inline void _unsetbit(size_t idx)noexcept requires(Alivebit_Cond)
    {
        if(_alivebits) _alivebits[idx/8]&=~(1<<idx%8);
    }

    //TODO handle partial bytes
    inline void _setbits(size_t begin, size_t end)noexcept requires(Alivebit_Cond);
    inline void _unsetbits(size_t begin, size_t end)noexcept requires(Alivebit_Cond);

    inline void _setdallbits()noexcept requires(Alivebit_Cond)
    {
        if(_alivebits)
            memset(_alivebits, 0xFF, _n_bytes(_dynmsz));
    }
    inline void _voidallbits()noexcept requires(Alivebit_Cond)
    {
        if(_alivebits)
            memset(_alivebits, 0, _n_bytes(_dynmsz));
    }
    inline bool _alive(size_t idx)const noexcept requires(Alivebit_Cond)
    {
        return !_alivebits||_alivebits[idx/8]&(1<<idx%8);
    }
    inline bool _allalive() const noexcept requires(Alivebit_Cond)
    {
        if(!_alivebits)
            return true;

        const size_t full_bytes = _dynmsz / 8;
        const size_t rem_bits   = _dynmsz % 8;

        for (size_t i = 0; i < full_bytes; ++i)
            if (_alivebits[i] != 0xFF)
                return false;

        if (rem_bits > 0)
        {
            uint8_t mask = (1 << rem_bits) - 1; // lower `rem_bits` set
            if ((_alivebits[full_bytes] & mask) != mask)
                return false;
        }

        return true;
    }
    inline bool _alldead() const noexcept requires(Alivebit_Cond)
    {
        if(!_alivebits)
            return false;

        const size_t full_bytes = _dynmsz / 8;
        const size_t rem_bits   = _dynmsz % 8;

        for (size_t i = 0; i < full_bytes; ++i)
            if (_alivebits[i] != 0x00)
                return false;

        if (rem_bits > 0)
        {
            uint8_t mask = (1 << rem_bits) - 1; // lower `rem_bits` set
            if ((_alivebits[full_bytes] & mask) != 0x00)
                return false;
        }

        return true;
    }
    
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

    //TODO align _alivebits to 8 or 16 bytes
    //mind cases where sizeof(T)>alignof(T)
    void _initcpct(size_t logical_size, bool trackbits)
    {
        const size_t allocsz=std::max(Mincpct, logical_size);
        _data=(T*)Alloc(allocsz*sizeof(T)+
            (Alivebit_Cond&&trackbits?_n_bytes(allocsz):0));
        if(!_data)
            throw std::bad_alloc();
        _dynmsz=logical_size;
        _capacity=allocsz;
        _alivebits=trackbits?(uint8_t*)(_data+_capacity):nullptr;
    }
    //user is responsible for initialization
    explicit storage(size_t dynamic_size=0)
        requires(dim==DYNAMIC&&!std::is_default_constructible_v<T>)
    {
        _initcpct(dynamic_size, true);
        if constexpr(Alivebit_Cond)if(_alivebits) _voidallbits();
    }
    //default construction
    explicit storage(size_t dynamic_size=0)
        requires(dim==DYNAMIC&&requires{T();})
    {
        _initcpct(dynamic_size, false);
        try{std::uninitialized_default_construct_n(_data, _dynmsz);}
        catch(...){Free(_data); throw;}
    }
    //copy construction
    explicit storage(size_t dynamic_size, const T& fillval)
        requires(dim==DYNAMIC&&requires{T(std::declval<const T&>());})
    {
        _initcpct(dynamic_size, false);
        try{std::uninitialized_fill_n(this->begin(), this->size(), fillval);}
        catch(...){Free(_data); throw;}
    }
    //custom construction
    template<typename...Args>
    explicit storage(size_t dynamic_size, Args&&...args)
        requires(dim==DYNAMIC&&requires{T(std::declval<Args>()...);}&&
            !std::is_same_v<T, Args...>)
    {
        _initcpct(dynamic_size, false);
        for(size_t i=0; i<dynamic_size; ++i) //potentially slow
            try{std::construct_at(_data+i, std::forward<Args>(args)...);}
            catch(...)
            {
                _destroy(_data, _data+i); 
                Free(_data);
                throw;
            }
    }
    
    //take ownership of preinitialized data
    //Precondition: `data` must point to `dynamic_size` Ts, if data is uninitialized,
    //initialized *must be* set to false.
    //`dynamic_size` must be >= 0, and data must point to at least Mincpct objects
    explicit storage(T* data, size_t dynamic_size, bool initialized=true)
        noexcept(!Alivebit_Cond||(Alivebit_Cond&&std::is_nothrow_destructible_v<T>))
        requires(dim==DYNAMIC)
    {
        assert(data!=nullptr);
        this->_data=data;
        this->_dynmsz=dynamic_size;
        this->_capacity=std::max(Mincpct, dynamic_size);
        if constexpr(!Alivebit_Cond)
            return;
        if(initialized)
            _alivebits=nullptr;
        else
        {
            _alivebits=(uint8_t*)Alloc(_n_bytes(_capacity));
            if(_alivebits==nullptr)
            {
                //memory is still raw, no need to destruct anything
                Free(_data);
                throw std::bad_alloc();
            }
            if constexpr(Alivebit_Cond) _voidallbits();
        }
    }
    //partially initialized data
    //`bits` points to user-provided init bits
    //will copy (dynamic_size+7)/8 bytes into bit buffer starting from
    //byte: bit_offset/8
    //bit_offset: a BIT (not byte) offset into the entire bits array,
    //copying begins from this bit. 
    //Must fulfill bit_offset+dynamic_size<number of bits in `bits`
    explicit storage(T* data, size_t dynamic_size, const uint8_t* bits, 
        size_t bit_offset=0)
        noexcept(!Alivebit_Cond||(Alivebit_Cond&&std::is_nothrow_destructible_v<T>))
        requires(dim==DYNAMIC)
    {
        assert(data&&(bits||dynamic_size==0));

        this->_data=data;
        this->_dynmsz=dynamic_size;
        this->_capacity=std::max(Mincpct, dynamic_size);
        if constexpr(!Alivebit_Cond)
            return;
        _alivebits=(uint8_t*)Alloc(_n_bytes(_capacity));
        if(_alivebits==nullptr)//bad Alloc, burn _data, we own it!
        {
            //destroy live elements
            if constexpr(!std::is_trivially_destructible_v<T>)
                for(size_t i=dynamic_size; i-->0;)
                    try
                    {
                        if(bits[(bit_offset+i)/8]&(1u<<((bit_offset+i)%8)))
                            (data+i)->~T();
                    }
                    catch(...){Free(_data); throw;}
            Free(_data);
            throw std::bad_alloc();
        }
        memset(_alivebits, 0x00, _n_bytes(_capacity));
        copy_bits(bits, bit_offset, dynamic_size,
            _alivebits);
    }

    //no-op land. this should shut up the compiler
    storage()noexcept requires(dim!=DYNAMIC)=default;

        /*COPY*/

    //use copy(). be explicit.
    storage(const storage&)=delete;
    
        /*MOVE*/
    
    storage(storage&&other)noexcept requires(dim==DYNAMIC)
    {
        _capacity=other._capacity;
        _dynmsz=other._dynmsz;
        _data=other._data;
        _alivebits=other._alivebits;
        
        other._data=nullptr;
        other._alivebits=nullptr;
        other._capacity=0;
        other._dynmsz=0;
    }

    /*OPERATORS*/

        /*OPERATOR=*/

    storage& operator=(const storage&)=delete;
    storage& operator=(storage&&)=delete;
    
    /*DATA*/
        
    inline void _destroy(T* begin, T* end)
        noexcept(std::is_nothrow_destructible_v<T>)
    {
        if constexpr (std::is_trivially_destructible_v<T>)
            return;
        if constexpr(!Alivebit_Cond)/*not tracking lifetimes*/
        {
            while(end!=begin)
                (--end)->~T();
            return;
        }
        else /*potentially partially initialized*/
        {
            if(!_alivebits) /*all alive*/
            {
                while(end!=begin)
                    (--end)->~T();
                return;
            }
            size_t idx=end-_data;//T* arithmetic, one past the end
            while(end!=begin)
            {
                --end; --idx;
                if(_alive(idx))
                {
                    end->~T();
                    _unsetbit(idx);
                }
            }
        }

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
        
        const size_t extra=Alivebit_Cond&&_alivebits?_n_bytes(newcpct):0;
        T* newaddr=(T*)Alloc(sizeof(T)*newcpct+extra);
        if(!newaddr) //Alloc fault
            return opres::MEM_ERR;
        
        const bool maybe_uninitialized=(Alivebit_Cond&&_alivebits);
        if constexpr (!Alivebit_Cond&&std::is_trivially_copyable_v<T>) //happy path
            memcpy(newaddr, _data, size()*sizeof(T));
        else if(size_t constructed=0; maybe_uninitialized) try//slow, branching
            {
                for(size_t i=0; i<_dynmsz; ++i)
                    if(_alive(i))
                    {
                        if constexpr(std::is_move_constructible_v<T>)
                        {
                            new (newaddr+i) T(std::move(this->at(i)));
                            constructed++;
                        }
                        else
                        {
                            new (newaddr+i) T(this->at(i));
                            constructed++;
                        }
                    }
            }
        catch(...)
        {
            for(size_t i=constructed; i!=0; --i)
                (newaddr+i-1)->~T();
            Free(newaddr);
            throw;
        }
        else try /*this->_data is fully constructed*/ 
        {
            if constexpr(std::is_move_constructible_v<T>)
                std::uninitialized_move(begin(), end(), newaddr);
            else
                std::uninitialized_copy(begin(), end(), newaddr);
        }
        catch(...)
        {
            Free(newaddr);
            throw;
        }
            
        if constexpr(Alivebit_Cond)if(_alivebits)
            //copy liveness metadata
            memcpy((void*)(newaddr+newcpct), _alivebits, 
                _n_bytes(_capacity));

        //this writes to _alivebits, so make sure to only call this
        //after properly copying the old array
        _destroy(this->begin(), this->end());
        
        if constexpr(Alivebit_Cond)if(_alivebits)
            if((void*)_alivebits!=(void*)(_data+_capacity))/*separately allocated*/
                Free(_alivebits);
        Free(_data);
        _alivebits=Alivebit_Cond&&_alivebits?(uint8_t*)(newaddr+newcpct):nullptr;
        _data=newaddr;
        _capacity=newcpct;

        return opres::SUCCESS;
    }
        
        /*RESIZE*/
        
    inline opres _resize_noinit(size_t newsize)
    noexcept(noexcept(rsvcpct(std::declval<size_t>()))&&
        std::is_nothrow_destructible_v<T>)
    requires(dim==DYNAMIC)
    {
        if(newsize==this->size())   //noop
            return opres::SUCCESS;
        if(newsize<this->size())    //shrink
        {
            _destroy(begin()+newsize, end());
            this->_dynmsz=newsize;
            return opres::SUCCESS;
        }
        size_t oldsize=size();
        auto res=rsvcpct(newsize); //check _capacity
        if(res==opres::SUCCESS)
            this->_dynmsz=newsize;
        //init new bits to false
        if constexpr(Alivebit_Cond)if(res==opres::SUCCESS&&_alivebits)
                for(size_t i=oldsize; i<newsize; ++i) _unsetbit(i);
        return res;
    }
    //performs no initialization
    inline opres resize(size_t newsize)
        noexcept(noexcept(_resize_noinit(std::declval<size_t>())))
        requires(dim==DYNAMIC&&!requires{T();})
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
        {
            std::uninitialized_default_construct_n(_data+oldsize, newsize-oldsize);
            if constexpr(Alivebit_Cond)if(_alivebits)
                for(size_t i=oldsize; i<newsize; ++i) _setbit(i);
        }
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
        {
            std::uninitialized_fill_n(this->begin()+oldsize, newsize-oldsize, fillval);
            if constexpr(Alivebit_Cond)if(_alivebits)
                for(size_t i=oldsize; i<newsize; ++i) _setbit(i);
        }
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
        {
            for(size_t i=oldsize; i<newsize; ++i)
            {
                std::construct_at(this->begin()+i, 
                    std::forward<Args>(args)...);
                if constexpr(Alivebit_Cond)
                    _setbit(i);
            }
        }
        return res;
    }

        /*COPY*/

    template<typename U=T, 
        size_t OthrMincpt=Mincpct, 
        memalloc_t OthrAlloc=Alloc, memfree_t OthrFree=Free>
    inline storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree>
    copy(size_t n_elements, size_t fromidx=0, opres*res=nullptr)const  
        noexcept(std::is_nothrow_constructible_v<U, const T&>)
        requires(dim==DYNAMIC&&requires{U(std::declval<const T&>());})
    {
        assert(n_elements+fromidx<=this->size());

        typedef storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree> ret_t;

        if(n_elements==0)
        {
            if(res) *res=opres::SUCCESS;
            return ret_t();//automatically allocs OthrMincpct, dynmsz==0
        }
        if(n_elements+fromidx>this->size())
        {
            if(res) *res=opres::BOUNDS_ERR;
            return ret_t();
        }
        
        //allocate raw memory
        U* resdata=(U*)OthrAlloc(std::max(n_elements, OthrMincpt)*sizeof(U));
        if(!resdata)
        {
            if(res) *res=opres::MEM_ERR;
            return ret_t();
        }
       
        if constexpr(Alivebit_Cond)if(_alivebits)
        {
            for(size_t i=fromidx; i<fromidx+n_elements; ++i)
                if(this->_alive(i))//copy construct alive elements*/
                    try
                    {
                        //we subtract fromidx so that destination starts at 0
                        std::construct_at(&resdata[i-fromidx],
                            this->at(i));
                    }
                    catch(...)//destroy every live object in reverse order
                    {
                        if constexpr(!std::is_trivially_destructible_v<U>)
                            for(size_t j=i-fromidx; j-->0;)//j=num ctd elems
                                try
                                {
                                    if(_alive(fromidx+j)) (resdata+j)->~U();
                                }
                                catch(...){OthrFree(resdata); throw;}
                        OthrFree(resdata);
                        throw;
                    }
            ret_t result((U*)resdata, (size_t)n_elements, _alivebits, fromidx);
            if(res) *res=opres::SUCCESS;
            return result;
        }
        //fully initialized data below
        
        //simplest path
        if constexpr(std::is_same_v<U, T>&&std::is_trivially_copyable_v<T>)
            memcpy(resdata, _data+fromidx, n_elements*sizeof(T));
        else
            try{std::uninitialized_copy_n(this->begin()+fromidx, n_elements, resdata);}
            //uninitialized_copy_n destroys elements automatically on throw
            catch(...){OthrFree(resdata); throw;}

        ret_t result((U*)resdata, (size_t)n_elements, true);

        if(res) *res=opres::SUCCESS;

        return result; //moved due to RVO
    }

    template<typename U=T, 
        size_t OthrMincpt=Mincpct, 
        memalloc_t OthrAlloc=Alloc, memfree_t OthrFree=Free>
    inline storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree>
    copy()const
        noexcept(std::is_nothrow_constructible_v<U, const T&>)
        requires(dim==DYNAMIC&&requires{U(std::declval<const T&>());})
    {
        return this->copy<U, OthrMincpt, OthrAlloc, OthrFree>(this->size());
    }
    
    //WARN: dst must point to n_elements+dst_startidx Us, and have NO 
    //overlap with [_data+src_startidx, _data+src_startidx+n_elements]
    //
    //initialize: whether the destination memory should be initialized 
    //(constructed) by this function 
    template<typename U=T>
    inline opres copyinto(
            U* dst,
            size_t n_elements,
            size_t dst_startidx=0,
            size_t src_startidx=0,
            bool initialize=false
            )const
        noexcept(std::is_nothrow_constructible_v<U, const T&>)
        requires(dim==DYNAMIC&&requires{U(std::declval<const T&>());})   
    {
        if constexpr(std::is_trivially_copyable_v<T>&&(std::is_same_v<T, U>))
            memcpy(dst+dst_startidx, this->begin()+src_startidx,
                n_elements*sizeof(U));
        else
        {
            if(initialize) //-->invoke copy ctor
                std::uninitialized_copy_n(this->begin()+src_startidx, n_elements, 
                    dst+dst_startidx);
            else //-->invoke copy assgn operator
                std::copy_n(this->begin()+src_startidx, n_elements, 
                    dst+dst_startidx);
        }
        return opres::SUCCESS;       
    }
    //initialize: whether the destination memory should be initialized 
    //(constructed) by this function 
    template<typename U=T, 
        size_t OthrMincpt=Mincpct, 
        memalloc_t OthrAlloc=Alloc, memfree_t OthrFree=Free>
    inline opres copyinto(
            storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree>&dst,
            size_t n_elements,
            size_t dst_startidx=0, 
            size_t src_startidx=0,
            bool initialize=false
            )const
        noexcept(std::is_nothrow_constructible_v<U, const T&>)
        requires(dim==DYNAMIC&&requires{U(std::declval<const T&>());})
    {
        assert(n_elements+src_startidx<=this->size());
        if(n_elements+src_startidx>this->size())
            return opres::BOUNDS_ERR;
        assert(n_elements+dst_startidx<=dst.size());
        if(n_elements+dst_startidx>dst.size())
            return opres::BOUNDS_ERR;

        opres res=copyinto(dst._data, n_elements, dst_startidx, src_startidx, 
            initialize);

        typedef storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree> ret_t;
        if(res==opres::SUCCESS)
            if constexpr(ret_t::Alivebit_Cond)if(dst._alivebits)
                for(size_t i=dst_startidx; i<dst_startidx+n_elements; ++i)
                    dst._setbit(i);
        return res;
    }
    
            /*MOVE*/
    
    template<typename U=T, 
        size_t OthrMincpt=Mincpct, 
        memalloc_t OthrAlloc=Alloc, memfree_t OthrFree=Free>
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
            return storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree>{};
        }
        //allocate raw memory
        U* resdata=(U*)OthrAlloc(std::max(n_elements, OthrMincpt)*sizeof(U));
        //initialize it
        std::uninitialized_move_n(this->begin()+fromidx, n_elements, resdata);
        //pass it
        storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree> 
            result((U*)resdata, (size_t)n_elements, true);
        
        if(res)
            *res=opres::SUCCESS;
        return result; //moved due to RVO
    }

    template<typename U=T,
        size_t OthrMincpt=Mincpct, 
        memalloc_t OthrAlloc=Alloc, memfree_t OthrFree=Free>
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
        memalloc_t OthrAlloc=Alloc, memfree_t OthrFree=Free>
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
        {
            std::uninitialized_move(start, start+n_elements, dst.begin()+dst_startidx);
            typedef storage<U, DYNAMIC, false, OthrMincpt, OthrAlloc, OthrFree> ret_t;
            if constexpr (ret_t::Alivebit_Cond)if(dst._alivebits)
                for(size_t i=dst_startidx; i<dst_startidx+n_elements; ++i)
                    dst._setbit(i);
        }
        else
        {
            static_assert((std::is_assignable_v<U, T&&>));
            std::move(start, start+n_elements, dst.begin()+dst_startidx);
        }            

        return opres::SUCCESS;
    }
    
    template<typename...Args>
    void inline construct_at(size_t idx, Args&&...args) 
        noexcept(std::is_nothrow_constructible_v<T, Args...>)
        requires(requires{T(std::declval<Args>()...);})
    {
        assert(idx<size()&&!_alive(idx));
        if constexpr(Alivebit_Cond)if(_alive(idx)) return;
        new (_data+idx) T(std::forward<Args>(args)...);
        if constexpr(Alivebit_Cond) _setbit(idx);
    }

    opres inline push_back(const T& obj)
    noexcept(noexcept(resize(std::declval<size_t>(), std::declval<const T&>())))
    requires(requires{resize(std::declval<size_t>(), std::declval<const T&>());})
    {
        return resize(size()+1, std::forward<const T&>(obj));
    }
    
    opres inline push_back(T&& obj)
    noexcept(noexcept(resize(std::declval<size_t>(), std::declval<T&&>())))
    requires(requires{resize(std::declval<size_t>(), std::declval<T&&>());})
    {
        return resize(size()+1, std::forward<T&&>(obj));
    }
    /*DESTRUCTOR*/
    
    ~storage()
        noexcept(std::is_nothrow_destructible_v<T>)
        requires(dim==DYNAMIC)
    {
        if(!this->_data)//invalid
            return;
        _destroy(this->begin(), this->end());
        if constexpr(Alivebit_Cond)if(_alivebits)
            if((void*)_alivebits!=(void*)(_data+_capacity))/*separately allocated*/
                Free(_alivebits);
        Free(this->_data);
    }
    ~storage()noexcept requires(dim!=DYNAMIC)=default;
};


}; //namespace aico
