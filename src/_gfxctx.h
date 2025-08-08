#pragma once


#include "aico/gfxctx.h"

#include "glad/glad.h"

namespace aico
{
    using ctx = gfxctx;

    struct ctx::buf_t::handle_t
    {
        GLuint value;
    };
    struct ctx::vtxlayout_t::handle_t
    {
        GLuint value;
    };
    struct ctx::shader_t::handle_t
    {
        GLuint value;
    };
    struct ctx::program_t::handle_t
    {
        GLuint value;
    };
    struct ctx::_impl
    {
        _impl(gfxconf_t);

        opres init(const sys::wndctx::info&)noexcept;
        
        gfxconf_t config;
        
        static GLuint& hndl(buf_t&)noexcept;
        static GLuint& hndl(vtxlayout_t&)noexcept;
        static GLuint& hndl(shader_t&)noexcept;
        static GLuint& hndl(program_t&)noexcept;

        static const GLuint& hndl(const buf_t&)noexcept;
        static const GLuint& hndl(const vtxlayout_t&)noexcept;
        static const GLuint& hndl(const shader_t&)noexcept;
        static const GLuint& hndl(const program_t&)noexcept;

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
        static constexpr GLenum gl(stageinfo::type t)noexcept
        {
            using T = stageinfo::type;
            switch (t) 
            {
                case(T::COMP): return GL_COMPUTE_SHADER;
                case(T::FRAG): return GL_FRAGMENT_SHADER;
                case(T::GEOM): return GL_GEOMETRY_SHADER;
                case(T::TESC): return GL_TESS_CONTROL_SHADER;
                case(T::TESE): return GL_TESS_EVALUATION_SHADER;
                case(T::VERT): return GL_VERTEX_SHADER;
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
