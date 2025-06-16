#pragma once

#include "gfxctx.h"

#include "glad/glad.h"

namespace aico
{
    struct gfxctx::buf_t::handle_t
    {
        GLuint value;
    };
    struct gfxctx::vtxlayout_t::handle_t
    {
        GLuint value;
    };
    struct gfxctx::_impl
    {
        _impl(gfxconf_t);

        opres init(const sys::wndctx::info&)noexcept;
        
        gfxconf_t config;
        
        static GLuint gethndl(const buf_t&)noexcept;
        static GLuint gethndl(const vtxlayout_t&)noexcept;
        
        static constexpr GLenum gl(attribinfo::type t)noexcept
        {
            using type = attribinfo::type;
            switch (t) 
            {
                case(type::FLOAT):      return      GL_FLOAT;
                case(type::HALF_FLT):   return GL_HALF_FLOAT;
                case(type::DOUBLE_FLT): return     GL_DOUBLE;
            }
        }
        static constexpr GLenum gl(vtxlayout_info::indexfmt t)noexcept
        {
            using fmt = vtxlayout_info::indexfmt;
            switch (t) 
            {
                case(fmt::U8):  return  GL_UNSIGNED_BYTE;
                case(fmt::U16): return GL_UNSIGNED_SHORT;
                case(fmt::U32): return   GL_UNSIGNED_INT;
            }
        }


        void logerr(const char* message);
        void loginf(const char* message);

        static void APIENTRY GLdebugproc([[maybe_unused]]GLenum source,
        GLenum type,
        [[maybe_unused]]GLuint id,
        GLenum severity, [[maybe_unused]]GLsizei length,
        const GLchar* message, [[maybe_unused]]const void* userParam);
    };
}
