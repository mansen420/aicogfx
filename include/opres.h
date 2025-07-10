#pragma once

#include <cstdint>

namespace aico
{
    enum class opres : uint8_t
    {
        SUCCESS, 
        FAILURE, 
        MEM_ERR, 
        BOUNDS_ERR, 
        ALIGN_ERR, 
        NO_HEAPS, 
        CONTEXT_CURRENT
    };
}
