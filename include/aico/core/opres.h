#pragma once

#include "aico/sys/opres.h"

namespace aico::core
{
    constexpr uint16_t OPRES_RANGE_BEGIN=sys::OPRES_RANGE_END;
    constexpr uint16_t OPRES_RANGE_END=OPRES_RANGE_BEGIN+0x0FFF;

    enum RESCAT: uint16_t
    {
        RESCAT_STORAGE=OPRES_RANGE_BEGIN,
    };
    enum RESCODE: uint16_t
    {
        RESCODE_SPRCPY,
    };

    /*attempted to copy from a sparce (partially initialized) vector*/
    inline constexpr opres_t SPRCPY= 
        opres_t{concat(RESCODE_SPRCPY, RESCAT_STORAGE)};
};
