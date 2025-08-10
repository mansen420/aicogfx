#pragma once

#include "aico/binutils.h"
#if AICO_DEBUG
#include "aico/time.h"
#endif

#include <cstdint>

namespace aico::sys
{
    enum: uint16_t
    {
        RESCAT_ANY      = 0,
        RESCAT_MALC     = 1,
        RESCAT_WNDCTX   = 2,
        RESCAT_GFXCTX   = 3,
    };
    enum: uint16_t
    {
        RESCODE_OK              = 0x0,
        RESCODE_GENERIC_ERR     = 0xFFFF,
        RESCODE_MEM_ERR         = 0xFFFE,
        RESCODE_BOUNDS_ERR      = 0xFFFD,
        RESCODE_INVALID_ARGS    = 0xFFDC,
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
    
    //the following named constants exist for user convenience
    inline constexpr opres_t OK=opres_t{concat(RESCODE_OK, RESCAT_ANY)};
    inline constexpr opres_t OOM=opres_t{concat(RESCODE_MEM_ERR, RESCAT_ANY)};
    //inline constexpr opres_t VM_MAP_FAILED;
    //inline constexpr opres_t THR_CREATE_FAILED;
}
