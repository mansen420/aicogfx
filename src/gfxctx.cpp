#include "gfxctx.h"
#include "_gfxctx.h"

#include "glad/glad.h"
#include "wndctx.h"
#include <sstream>

aico::gfxctx::gfxctx(gfxconf_t config) : implptr(new _impl(config))
{};
aico::gfxctx::_impl::_impl(gfxconf_t config) : config(config) {}
aico::gfxctx::~gfxctx()noexcept{delete implptr;}
aico::opres aico::gfxctx::_init(const sys::wndctx::info& info)noexcept
{return implptr->init(info);}
aico::gfxctx::_impl* aico::gfxctx::getimpl()const noexcept{return implptr;}

void aico::gfxctx::_impl::logerr(const char* msg)
{
    config.errlog << msg;
}
void aico::gfxctx::_impl::loginf(const char* msg)
{
    config.inflog << msg;
}

void set_debug_callback(GLDEBUGPROC debugfn, void* usrparam)noexcept;
aico::opres aico::gfxctx::_impl::init(const sys::wndctx::info& wndinfo)noexcept
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

void APIENTRY aico::gfxctx::_impl::GLdebugproc([[maybe_unused]]GLenum source, GLenum type,
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
    auto ctxptr = (aico::gfxctx::_impl*) userParam;
    
    //TODO perhaps it would be better to store this externally, avoid init cost
    std::ostringstream oss;
    oss << "GL DEBUG: [" << severity_str << "][" << type_str << "]: " << 
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
