#pragma once

#include "malc.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <type_traits>
#include <utility>

namespace aico::sys
{
    template<typename T, typename... Args>
    requires(requires{T(std::declval<Args>()...);}&&!std::is_void_v<T>&&requires{~T();})
    inline T* alloctr (size_t count=1, Args&&...args)
    {
        opres res;
        auto addr=(T*)malc(sizeof(T)*count, 
            alignof(T)>=alignof(void*)?alignof(T):alignof(void*), &res);
        if(!(res==opres::SUCCESS&&addr)) throw std::bad_alloc();
        size_t ctd=0;
        for(T* it=addr; it<addr+count; ++it)
            try{new (it) T(std::forward<Args>(args)...); ++ctd;}
            catch(...)
            {
                for(T* end=addr+ctd; end!=addr;)
                    (--end)->~T();
                rel(addr);
                throw;
            }
        return addr;
    }
    
    //WARN: this function assumes memory is properly initialized
    template<typename T>
    requires(!std::is_void_v<T>&&requires{~T();})
    inline void dtrel(T* addr) noexcept(std::is_nothrow_destructible_v<T>)
    {
        if constexpr(std::is_trivially_destructible_v<T>)
        {
            rel(addr);
            return;
        }
        auto hdr=gethdr(addr);
        if(!hdr) return; //the fuck?
        //XXX this is fragile, relies on hdr_t too much, consider storing obj count
        //in metadata directly
        size_t nrTs=(hdr->bytes-((uintptr_t)addr-(uintptr_t)hdr->base))/sizeof(T);
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
