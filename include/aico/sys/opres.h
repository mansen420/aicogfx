#pragma once

#include "aico/opres.h"

namespace aico::sys
{
    constexpr uint16_t OPRES_RANGE_BEGIN=aico::OPRES_RANGE_END;
    constexpr uint16_t OPRES_RANGE_END=0x1000;//excluded
    enum RESCAT: uint16_t
    {
        RESCAT_MALC=OPRES_RANGE_BEGIN,  //aico's malc
        RESCAT_NATALC,                  //system-native allocator 
    };
    enum RESCODE: uint16_t
    {
        RESCODE_NOHEAPS,
        RESCODE_ALIGNERR=0xFFFF,
    };
    

    inline constexpr opres_t NOHEAPS=
        opres_t{concat(RESCODE_NOHEAPS, RESCAT_MALC)};
    inline constexpr opres_t SALCERR=
        opres_t{concat(RESCODE_MEMERR, RESCAT_NATALC)};
    inline constexpr opres_t ALIGNERR=
        opres_t{concat(RESCODE_ALIGNERR, RESCAT_MALC)};
}
