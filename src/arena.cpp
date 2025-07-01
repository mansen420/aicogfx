#include "arena.h"
#include <cstdlib>
#include <new>

arena::arena(std::size_t size)
{
    buffer = static_cast<std::byte*>(std::malloc(size));
    if (!buffer) throw std::bad_alloc();
    capacity = size;
    offset = 0;
}

arena::~arena()
{
    std::free(buffer);
}

void* arena::alloc(std::size_t size, std::size_t align)
{
    std::size_t current = reinterpret_cast<std::uintptr_t>(buffer + offset);
    std::size_t aligned = (current + align - 1) & ~(align - 1);
    std::size_t newOffset = aligned - reinterpret_cast<std::uintptr_t>(buffer) + size;

    if (newOffset > capacity)
        return nullptr; // out of memory

    void* ptr = buffer + (aligned - reinterpret_cast<std::uintptr_t>(buffer));
    offset = newOffset;
    return ptr;
}

void arena::reset()
{
    offset = 0;
}
