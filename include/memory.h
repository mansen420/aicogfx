#pragma once

#include <cstddef>
#include <cstdlib>
#include <type_traits>

namespace aico::sys
{
    template<typename T=void>
        requires(!std::is_void_v<T>)
            [[nodiscard]] inline T* memalloc(
        size_t bytes, 
        size_t alignment=alignof(T))noexcept
    {
        return (T*)malloc(bytes);
    }

    [[nodiscard]] inline void* memalloc(size_t bytes,
        size_t alignment=alignof(max_align_t))noexcept
    {
        return malloc(bytes);
    }

    template<typename T>
        requires(!std::is_void_v<T>)
    [[nodiscard]] inline T* cmemalloc(size_t count)noexcept
    {
        return memalloc<T>(sizeof(T)*count);
    }

    inline void memfree(void* addr)noexcept
    {
        free(addr);
    }

    template<typename T, typename... Args>
        requires(requires{T(std::declval<Args>()...);})
    inline T* alloctr (size_t count=1, Args&&...args);
    template<typename  T>
    inline void dtrfree(T* addr);
}
