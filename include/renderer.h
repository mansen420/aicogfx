#pragma once
#include "wndctx.h"

namespace aicogfx::sys
{
    struct renderer_t
    {
        const wndctx::render_callback fnc;
        void* const stateptr;
    };
    extern renderer_t flashing_red;
}
