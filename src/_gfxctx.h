#pragma once

#include "gfxctx.h"

#include "glad/glad.h"

namespace aico
{
    struct gfxctx::buf_t::handle_t
    {
        GLuint value;
        operator GLuint*(){return &value;}
    };
    struct gfxctx::_impl
    {
        _impl(gfxconf_t);

        opres init(const sys::wndctx::info&)noexcept;
        
        gfxconf_t config;
        
        buf_t::handle_t gethndl(const buf_t&)const noexcept;


        void logerr(const char* message);
        void loginf(const char* message);

        static void APIENTRY GLdebugproc([[maybe_unused]]GLenum source,
        GLenum type,
        [[maybe_unused]]GLuint id,
        GLenum severity, [[maybe_unused]]GLsizei length,
        const GLchar* message, [[maybe_unused]]const void* userParam);
    };
}
