#pragma once

#include "aico/core/opres.h"

namespace aico::gpu
{
    constexpr uint16_t OPRES_RANGE_BEGIN=core::OPRES_RANGE_BEGIN;
    constexpr uint16_t OPRES_RANGE_END=OPRES_RANGE_BEGIN+0x0FFF;

    enum RESCAT: uint16_t
    {
        RESCAT_WINDOW=OPRES_RANGE_BEGIN,
    };
    enum RESCODE: uint16_t
    {
        RESCODE_GLCTX_ALREADY_CURRENT,
    };

    inline constexpr opres_t GLCTX_CURRENT=
        opres_t{concat(RESCODE_GLCTX_ALREADY_CURRENT, OPRES_RANGE_BEGIN)};
};
