#include "gfxctx.h"
#include "_gfxctx.h"

#include "glad/glad.h"
#include "opres.h"
#include "wndctx.h"

#include <sstream>

using namespace aico;
using ctx = gfxctx;

ctx::vtxlayout_t::vtxlayout_t(ctx::vtxlayout_info info): _info(info), _hnd(new handle_t)
{}
ctx::vtxlayout_t ctx::make_vtxlayout(vtxlayout_info info)const noexcept
{
    vtxlayout_t layout(info);
    glCreateVertexArrays(1, &layout._hnd->value);
    auto vaobj = _impl::gethndl(layout);
    for(const auto& bind : layout._info.buffers)
    {
        glVertexArrayVertexBuffer(vaobj, bind.idx, 
            _impl::gethndl(bind.buffer), bind.offset,
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
        glVertexArrayElementBuffer(vaobj, _impl::gethndl(buf_fmt->first));
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
ctx::buf_t ctx::bufalloc(bufinfo info, const void* data)const noexcept
{
    //if something failed, let the driver scream, i guess
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
    glDeleteBuffers(1, &buffer._hnd->value);
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

GLuint ctx::_impl::gethndl(const ctx::buf_t&x)noexcept{return x._hnd->value;}
GLuint ctx::_impl::gethndl(const ctx::vtxlayout_t&x)noexcept
{return x._hnd->value;}

ctx::gfxctx(gfxconf_t config) : implptr(new _impl(config)){}
ctx::_impl::_impl(gfxconf_t config) : config(config) {}
ctx::~gfxctx()noexcept{delete implptr;}
opres ctx::_init(const sys::wndctx::info& info)noexcept
{return implptr->init(info);}
ctx::_impl* ctx::getimpl()const noexcept{return implptr;}

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
