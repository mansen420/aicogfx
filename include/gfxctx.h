#pragma once

#include "opres.h"
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
    public:
        friend struct sys::wndctx;

        gfxctx(const gfxctx&) = delete;
        gfxctx(gfxctx&&) noexcept;
        ~gfxctx() noexcept;
        
        struct bufinfo
        {
            size_t size;
            size_t stride;
        };
        struct buf
        {
            bufinfo info();
            friend struct gfxctx;
        private:
            bufinfo _info;

            buf(const bufinfo);

            struct handle_t;
            handle_t* hnd;
        };

        [[nodiscard]]buf bufalloc(bufinfo, const void*)const noexcept;
        void freebuf(buf&)const noexcept;
        opres bufdata(const buf&, const void* data, size_t size, size_t buf_offset)const noexcept;

    private:
        gfxctx(gfxconf_t);

        opres _init(const sys::wndctx::info&) noexcept;
        
        struct _impl;
        _impl* implptr;
    public:
        [[nodiscard]]_impl* getimpl()const noexcept;
    };
}
