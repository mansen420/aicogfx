#pragma once

#include <cstdint>

namespace aico
{
    enum class opres : uint8_t
    {
        SUCCESS, FAILURE, OOM, CONTEXT_CURRENT
    };
}
