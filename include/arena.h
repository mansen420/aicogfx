#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>

class arena {
public:
    arena(std::size_t size);
    ~arena();

    void* alloc(std::size_t size, std::size_t align = alignof(std::max_align_t));
    void reset(); // wipes the arena

private:
    std::byte* buffer = nullptr;
    std::size_t capacity = 0;
    std::size_t offset = 0;
};
