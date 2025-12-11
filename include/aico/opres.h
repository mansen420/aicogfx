#pragma once

#include "aico/binutils.h"
#if AICO_DEBUG
#include "aico/time.h"
#endif

#include <cstdint>

namespace aico
{
    /*
     * common:  [0x0000, 0x0100]
     * sys:     [0x0100, 0x0FFF]
     * core:    [0x1000, 0x1FFF]
     * gpu:     [0x2000, 0x2FFF]
     * gfx:     [0x3000, 0x3FFF]
     * reserve: [0x4000, 0x7FFF] //aico-reserved
     * user:    [0x8000, 0xFFFF] //user domain
     * */
    constexpr uint16_t OPRES_RANGE_BEGIN=0x0000;
    constexpr uint16_t OPRES_RANGE_END=0x0100;//excluded
    enum RESCAT: uint16_t
    {
        RESCAT_ANY=OPRES_RANGE_BEGIN,
    };
    enum: uint16_t
    {
        RESCODE_OK,
        RESCODE_IDXERR,
        RESCODE_MEMERR,
        RESCODE_INVARG,
        RESCODE_GENERR=0x00FF,
    };
    
    [[nodiscard]] inline constexpr uint16_t rescode(uint32_t combined)noexcept
    {return lo16(combined);}
    [[nodiscard]] inline constexpr uint16_t rescat(uint32_t combined)noexcept
    {return hi16(combined);}

#if AICO_DEBUG
    struct opres_t
    {
        uint16_t code=RESCODE_OK;
        uint16_t cat=RESCAT_ANY;
        const char* msg=nullptr;
        const char* file=__FILE__;
        uint32_t line=__LINE__;
        uint64_t time=0; //nanoseconds since aico, 0-->unspecified
        
        constexpr opres_t(uint32_t combined, const char* msg=nullptr, uint64_t time=0): 
        code{rescode(combined)},
        cat{rescat(combined)},
        msg{msg},
        time{time}
        {}
        constexpr bool operator==(const opres_t& other)const noexcept
        {
            return this->cat==other.cat&&this->code==other.code;
        }
    };
    [[nodiscard]]inline opres_t 
    opres(uint16_t code, uint16_t cat, const char* msg=nullptr)noexcept
    {
        return opres_t{concat(code, cat), 
            msg, aico_time()};
    }

    [[nodiscard]] inline constexpr
    uint16_t rescode(opres_t res)noexcept{return res.code;}
    [[nodiscard]] inline constexpr
    uint16_t rescat(opres_t res)noexcept{return res.cat;}
#else
    typedef uint32_t opres_t;
    [[nodiscard]]inline constexpr opres_t 
    opres(uint16_t code, uint16_t cat, const char* msg=nullptr)noexcept
    {
        return concat(code, cat);
    }
#endif

    inline constexpr opres_t OK=
        opres_t{concat(RESCODE_OK, RESCAT_ANY)};
    inline constexpr opres_t FAIL=
        opres_t{concat(RESCODE_GENERR, RESCAT_ANY)};
    inline constexpr opres_t MEMERR=
        opres_t{concat(RESCODE_MEMERR, RESCAT_ANY)};
    inline constexpr opres_t IDXERR=
        opres_t{concat(RESCODE_IDXERR, RESCAT_ANY)};
    inline constexpr opres_t INVARG=
        opres_t{concat(RESCODE_INVARG, RESCAT_ANY)};
}
