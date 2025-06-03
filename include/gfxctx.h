#pragma once

#include "wndctx.h"

#include <iostream>
#include <ostream>

namespace aico
{
    struct gfxconf_t
    {
        std::ostream& errlog = std::cerr;
        std::ostream& inflog = std::cout;
    };
    struct gfxctx
    {
        friend struct sys::wndctx;
        gfxctx(const gfxctx&) = delete;
        gfxctx(gfxctx&&);
        ~gfxctx();
    private:
        gfxctx(gfxconf_t);
        opres _init(const sys::wndctx::info&);
        struct _impl;
        _impl* implptr;
    };
}
