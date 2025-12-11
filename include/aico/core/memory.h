#pragma once

#include <cassert>
#include <type_traits>
#include <utility>

#include "aico/sys/memory.h"
#include "aico/core/opres.h"
#include "aico/sys/malc.h"

namespace aico::core
{
    //templated version of malc, returns typed pointer to raw memory
    template<typename T> requires(!std::is_void_v<T>)
    inline T* malc(size_t count, size_t alignment=alignof(std::max_align_t)
        , opres_t* res=nullptr)
    {
        return (T*)sys::malc(sizeof(T)*count, alignment, res);
    }

    template<typename T, typename...Args>
    requires(std::is_constructible_v<T, Args...>)
    void construct(T* addr, size_t n, Args&&...args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        if(sizeof...(Args)==0&&std::is_trivially_default_constructible_v<T>)
            return;

        if constexpr (sizeof...(Args) == 1&&
            std::is_rvalue_reference_v<Args&&...>)
            assert(n == 1 && "construct(T*, n, T&&) with n>1 is"\
                "not allowed; use fill_construct or pass lvalue");
        for(size_t i=0; i<n; ++i)
            try
            {
                if constexpr(sizeof...(Args)==0)
                    new(addr+i) T;
                else
                    new(addr+i) T(std::forward<Args>(args)...);
            }
            catch(...)
            {
                for(size_t j=i; j!=0; --j)
                    (addr+(j-1))->~T();
                throw;
            };
    }
    template<typename T>
    void fill_construct(T* addr, size_t n, const T& fillval)
    noexcept(std::is_nothrow_constructible_v<T, const T&>) 
    requires(std::is_constructible_v<T, const T&>)
    {
        if constexpr(std::is_trivially_copy_constructible_v<T>)
        {
            memfill(addr, &fillval, sizeof(fillval), n);
            return;
        }
        construct(addr, n, fillval);
    }
    template<typename T>
    void move_construct(T* addr, T&& other)
    noexcept(std::is_nothrow_move_constructible_v<T>)
    requires(std::is_move_constructible_v<T>)
    {
        new (addr) T(std::forward<T>(other));
    }
    template<typename T>
    void move_range(T* dst, T* src, size_t n);//TODO
    
    template<typename T, typename... Args>
    requires(std::is_constructible_v<T, Args...>&&!std::is_void_v<T>)
    inline T* alloctr (size_t count=1, Args&&...args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        if(count==0) return nullptr;
        opres_t res;
        auto addr=malc<T>(sizeof(T)*count, 
                alignof(T)>=alignof(void*)?
                alignof(T):
                alignof(void*), 
                &res);
        if(!(res==OK&&addr)) return nullptr;
        
        try{construct(addr, count, std::forward<Args>(args)...);}
        catch(...){rel(addr); throw;}
        return addr;
    }
    
    //WARN: this function assumes memory is properly initialized
    template<typename T>
    requires(!std::is_void_v<T>&&std::is_destructible_v<T>)
    inline void dtrel(T* addr) noexcept(std::is_nothrow_destructible_v<T>)
    {
        if constexpr(std::is_trivially_destructible_v<T>)
        {
            rel(addr);
            return;
        }
        size_t nrTs=nr_objs(addr);
        for(T* end=addr+nrTs; end!=addr;)
            try{(--end)->~T();}
            catch(...)
            {
                rel(addr); //dumb user, eat memory leaks
                throw;
            }
        rel(addr);
    }
}
