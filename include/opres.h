#pragma once

#include "stdint.h"

namespace aico
{
    enum class opres : uint32_t
    {
        SUCCESS, FAILURE, CONTEXT_CURRENT
    };
}
