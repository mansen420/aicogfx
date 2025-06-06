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
        gfxctx(gfxctx&&) noexcept;
        ~gfxctx() noexcept;
        
    private:
        gfxctx(gfxconf_t);
        
        opres _init(const sys::wndctx::info&) noexcept;
        
        struct _impl;
        _impl* implptr;
    public:
        [[nodiscard]]_impl* getimpl()const noexcept;
    };
}
