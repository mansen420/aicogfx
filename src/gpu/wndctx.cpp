#include "aico/gpu/opres.h"
#include "aico/gpu/wndctx.h"
#include "aico/gpu/gfxctx.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <atomic>
#include <cstddef>
#include <cstdint>

struct aico::gpu::wndctx::_impl
{
    std::atomic<bool> kill_loop;
    std::atomic<bool> looping;
    _impl(int width, int height, const char* title, uint32_t flags) : 
        kill_loop(false),
        looping(false)
    {
        if(glfwGetCurrentContext() != NULL)//another context is already current on calling thread
            throw GLCTX_CURRENT;
        
        if(flags & bits::DEBUGCTX)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

        winptr = glfwCreateWindow(width, height, title, nullptr, nullptr);

        if(!winptr)
            throw FAIL;

        glfwMakeContextCurrent(winptr);

        if(!gladLoadGL())
        {
            glfwDestroyWindow(winptr);
            throw FAIL;
        }

        //validate ctx
        if(flags & bits::DEBUGCTX)
        {
            GLint GLflags;
            glGetIntegerv(GL_CONTEXT_FLAGS, &GLflags);
            if(!(GLflags & GL_CONTEXT_FLAG_DEBUG_BIT))
                throw FAIL;
        }
    }
    void loop(render_callback fnc, const frameinfo& framedata, void* usrdata)
    {
        kill_loop.store(false);
        looping.store(true);
        while(!(glfwWindowShouldClose(winptr) || kill_loop.load()))
        {
            if(fnc != nullptr)
                fnc(framedata, gfxctxptr, usrdata);

            glfwSwapBuffers(winptr);
            glfwPollEvents();
        }
        looping.store(false);
    }
    ~_impl()
    {
        if(gfxctxptr != nullptr)
            delete gfxctxptr;
        glfwDestroyWindow(winptr);
    }
    gfxctx* gfxctxptr = nullptr;
private:
    GLFWwindow* winptr;
};

aico::gpu::wndctx::wndctx(int width, int height, const char* title, 
renderer_t renderer, uint32_t flags) : 
    renderfnc(renderer.fnc), 
    render_initfnc(renderer.initfnc),
    render_termfnc(renderer.termfnc),
    stateptr(renderer.stateptr),
    _info{width, height, title, flags},
    implptr(new _impl(width, height, title, flags))
{}

aico::gpu::wndctx::~wndctx()noexcept{delete implptr;}

aico::gpu::wndctx::wndctx(wndctx&& other)noexcept : renderfnc(other.renderfnc),
stateptr(other.stateptr), _info(other._info), implptr(other.implptr){other.implptr = nullptr;}

aico::gpu::gfxctx* aico::gpu::wndctx::makegfxctx(gfxconf_t conf, opres_t* res)const noexcept
{
    if(implptr->gfxctxptr != nullptr)
        delete implptr->gfxctxptr; //safely destroy old ctx
    implptr->gfxctxptr = new gfxctx(conf);
    opres_t status = implptr->gfxctxptr->_init(_info);
    if(res != nullptr)
        *res = status;
    return implptr->gfxctxptr;
}
void aico::gpu::wndctx::interrupt() noexcept{implptr->kill_loop.store(true);}
void aico::gpu::wndctx::loop()
{
    if(render_initfnc != nullptr)
    {
        auto res = render_initfnc(implptr->gfxctxptr, stateptr);
        if(res == FAIL)
            return;
    }
    implptr->loop(renderfnc, _framedata, stateptr);
    if(render_termfnc != nullptr)
        render_termfnc(implptr->gfxctxptr, stateptr);
}
bool aico::gpu::wndctx::looping()const noexcept{return implptr->looping.load();}
