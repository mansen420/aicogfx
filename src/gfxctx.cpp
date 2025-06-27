#include "gfxctx.h"
#include "_gfxctx.h"

#include "glad/glad.h"
#include "opres.h"
#include "wndctx.h"

#include <cassert>
#include <sstream>
#include <cstring>

using namespace aico;
using ctx = gfxctx;

const GLuint& ctx::_impl::hndl(const ctx::buf_t&x)noexcept{return x._hnd->value;}
const GLuint& ctx::_impl::hndl(const ctx::vtxlayout_t&x)noexcept{return x._hnd->value;}
const GLuint& ctx::_impl::hndl(const ctx::shader_t&x)noexcept{return x._hnd->value;}
const GLuint& ctx::_impl::hndl(const ctx::program_t&x)noexcept{return x._hnd->value;}

GLuint& ctx::_impl::hndl(ctx::buf_t&x)noexcept{return x._hnd->value;}
GLuint& ctx::_impl::hndl(ctx::vtxlayout_t&x)noexcept{return x._hnd->value;}
GLuint& ctx::_impl::hndl(ctx::shader_t&x)noexcept{return x._hnd->value;}
GLuint& ctx::_impl::hndl(ctx::program_t&x)noexcept{return x._hnd->value;}


ctx::shader_t::shader_t(ctx::stageinfo info): _type(info.T), _hnd(new handle_t){}
ctx::shader_t ctx::compile(ctx::stageinfo info, opres* res)const noexcept
{
    int length = info.length == -1? (int)strlen(info.src) : info.length;
    assert(length > 0);

    shader_t stg(info);
    if(!stg._hnd)
    {
        if(res)
            *res = opres::FAILURE; //alloc failure
        return stg;
    }
    _impl::hndl(stg) = glCreateShader(_impl::gl(stg._type));
    if(_impl::hndl(stg) == 0)
    {
        if(res)
            *res = opres::FAILURE; //GL error
        return stg;
    }
    glShaderSource(_impl::hndl(stg), 1, &info.src, 
        info.length == -1 ? nullptr : &info.length);
    glCompileShader(_impl::hndl(stg));
    GLint status;
    glGetShaderiv(_impl::hndl(stg), GL_COMPILE_STATUS,
        &status);
    if(status == GL_FALSE)
    {
        if(res)
            *res = opres::FAILURE; //compilation failure
        return stg;
    }
    return stg;
}
void ctx::free(shader_t& stg)const noexcept
{
    if(!stg._hnd)
        return;
    glDeleteShader(_impl::hndl(stg));
    delete stg._hnd;
    stg._hnd=nullptr;
}
ctx::program_t::program_t(): _hnd(new handle_t){}
ctx::program_t ctx::link(const std::vector<shader_t>& stages, opres* res)const noexcept
{
    program_t prog;
    if(!_impl::hndl(prog))
    {
        if(res)
            *res = opres::FAILURE;//alloc failure
        return prog;
    }
    _impl::hndl(prog)=glCreateProgram();
    for(const auto& stage : stages)
        glAttachShader(_impl::hndl(prog), _impl::hndl(stage));
    glLinkProgram(_impl::hndl(prog));
    GLint status;
    glGetProgramiv(_impl::hndl(prog), GL_LINK_STATUS,
        &status);
    if(status==GL_FALSE)
    {
        if(res)
            *res=opres::FAILURE;//link error
        return prog;
    }
    return prog;
}
void ctx::free(program_t& prog)const noexcept
{
    if(!prog._hnd)
        return;
    glDeleteProgram(_impl::hndl(prog));
    delete prog._hnd;
    prog._hnd=nullptr;
}
opres ctx::bind(program_t prog)const noexcept
{
    //HACK: just assume prog is valid for now
    glUseProgram(_impl::hndl(prog));
    return opres::SUCCESS;
}

ctx::vtxlayout_t::vtxlayout_t(ctx::vtxlayout_info info): _info(info), _hnd(new handle_t){}
ctx::vtxlayout_t ctx::make_vtxlayout(vtxlayout_info info)const noexcept
{
    vtxlayout_t layout(info);
    glCreateVertexArrays(1, &layout._hnd->value);
    auto vaobj = _impl::hndl(layout);
    for(const auto& bind : layout._info.buffers)
    {
        glVertexArrayVertexBuffer(vaobj, bind.idx, 
            _impl::hndl(bind.buffer), bind.offset,
                (int)bind.buffer._info.stride);
    }
    for(const auto& attrib : layout._info.attribs)
    {
        glEnableVertexArrayAttrib(vaobj, attrib.idx);
        glVertexArrayAttribFormat(vaobj, attrib.idx, (int)attrib.size,
            _impl::gl(attrib.T), GL_FALSE,
                attrib.reloffst);
        glVertexArrayAttribBinding(vaobj, attrib.idx, 
            attrib.bindidx);
    }
    if(const auto& buf_fmt = layout._info.indexbuf_fmt; buf_fmt.has_value())
        glVertexArrayElementBuffer(vaobj, _impl::hndl(buf_fmt->first));
    return layout;
}
opres ctx::bind(const vtxlayout_t& layout)const noexcept
{
    glBindVertexArray(layout._hnd->value);
    return opres::SUCCESS;
}
void ctx::free(vtxlayout_t& layout)const noexcept
{
    if(!layout._hnd)
        return;
    glDeleteVertexArrays(1, &layout._hnd->value);
    delete layout._hnd;
    layout._hnd = nullptr;
}

ctx::buf_t::buf_t(ctx::bufinfo info): _info(info), _hnd(new handle_t){}
ctx::buf_t ctx::bufalloc(bufinfo info, const void* data, opres* res)const noexcept
{
    //if something failed, let the driver scream, i guess
    //TODO return error states via res
    buf_t buffer(info);
    glCreateBuffers(1, &buffer._hnd->value);
    glNamedBufferStorage(buffer._hnd->value, (long)buffer._info.size, data,
        GL_DYNAMIC_STORAGE_BIT);
    return buffer;
}
void ctx::free(buf_t& buffer)const noexcept 
{
    if(!buffer._hnd)
        return;
    glDeleteBuffers(1, &_impl::hndl(buffer));
    delete buffer._hnd;
    buffer._hnd = nullptr;
}
opres ctx::bufdata(const buf_t& buffer, const void* data, size_t size,
    size_t buf_offset)const noexcept
{
    if(buf_offset + size > buffer._info.size)
        return opres::FAILURE;
    glNamedBufferSubData(buffer._hnd->value, buf_offset, size, data);
    return opres::SUCCESS;
}

ctx::gfxctx(gfxconf_t config) : implptr(new _impl(config)){}
ctx::_impl::_impl(gfxconf_t config) : config(config) {}
ctx::~gfxctx()noexcept{delete implptr;}
opres ctx::_init(const sys::wndctx::info& info)noexcept{return implptr->init(info);}
ctx::_impl* ctx::getimpl()noexcept{return implptr;}

void ctx::_impl::logerr(const char* msg)
{
    config.errlog << msg;
}
void ctx::_impl::loginf(const char* msg)
{
    config.inflog << msg;
}

void set_debug_callback(GLDEBUGPROC debugfn, void* usrparam)noexcept;
opres ctx::_impl::init(const sys::wndctx::info& wndinfo)noexcept
{
    if(wndinfo.flags & sys::wndctx::bits::DEBUGCTX)
    {
        GLint GLflags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &GLflags);
        if(!(GLflags & GL_CONTEXT_FLAG_DEBUG_BIT))
            return opres::FAILURE;
        set_debug_callback(GLdebugproc, this);
    }
    return opres::SUCCESS;
}

void APIENTRY ctx::_impl::GLdebugproc([[maybe_unused]]GLenum source, GLenum type,
    [[maybe_unused]]GLuint id,
    GLenum severity, [[maybe_unused]]GLsizei length,
    const GLchar* message, [[maybe_unused]]const void* userParam)
{
    const char* type_str =
        type == GL_DEBUG_TYPE_ERROR ? "ERROR" :
        type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR ? "DEPRECATED" :
        type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR ? "UNDEFINED" :
        type == GL_DEBUG_TYPE_PORTABILITY ? "PORTABILITY" :
        type == GL_DEBUG_TYPE_PERFORMANCE ? "PERFORMANCE" :
        "OTHER";

    const char* severity_str =
        severity == GL_DEBUG_SEVERITY_HIGH ? "HIGH" :
        severity == GL_DEBUG_SEVERITY_MEDIUM ? "MEDIUM" :
        severity == GL_DEBUG_SEVERITY_LOW ? "LOW" : "NOTIFICATION";
    
    //TODO configure this so that it uses errlog and inflog appropriately
    auto ctxptr = (ctx::_impl*) userParam;
    
    //TODO perhaps it would be better to store this externally, avoid init cost
    std::ostringstream oss;
    oss << "GL DEBUG: [" << severity_str << "][" << type_str << "]\n" << 
        message << "\n";
    ctxptr->logerr(oss.str().c_str());

    if (severity == GL_DEBUG_SEVERITY_HIGH || type == GL_DEBUG_TYPE_ERROR) 
    {
        ctxptr->logerr("OpenGL error, you probably messed up. TERM.\n");
        // optionally abort here
    }
}

void set_debug_callback(GLDEBUGPROC debugfn, void* usrparam)noexcept
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageCallback(debugfn, usrparam);

    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
        GL_DONT_CARE, 0, nullptr, GL_TRUE);
}
