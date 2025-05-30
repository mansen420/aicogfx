#pragma once
#include <stdint.h>
namespace aicogfx
{ 
    struct engctx
    {
        engctx();
        
        enum bits
        {
            INIT = 1 << 0
        };
        
        const uint32_t& flags;
        
        ~engctx()noexcept;
    private:
        struct _impl;
        _impl* implptr;
    };
};
