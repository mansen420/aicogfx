#include "wndctx.h"
#include "engctx.h"
#include "sysinit.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <atomic>
#include <cstddef>

static uint32_t engine_flags = 0;

aico::engctx::engctx() : flags(engine_flags)
{
    if(auto status = sys::init(); status != opres::SUCCESS)
        throw status;
}
aico::engctx::~engctx()noexcept
{
    sys::terminate();
}

aico::opres aico::sys::init()
{
    if(!glfwInit())
        return opres::FAILURE;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    engine_flags = engine_flags | engctx::bits::INIT;

    return opres::SUCCESS;
}

void aico::sys::terminate() noexcept
{
    glfwTerminate();
}

struct aico::sys::wndctx::_impl
{
    std::atomic<bool> kill_loop;
    std::atomic<bool> looping;
    _impl(int width, int height, const char* title) : kill_loop(false),
    looping(false)
    {
        if(glfwGetCurrentContext() != NULL)//another context is already current on calling thread
            throw opres::CONTEXT_CURRENT;

        winptr = glfwCreateWindow(width, height, title, nullptr, nullptr);

        if(!winptr)
            throw opres::FAILURE;

        glfwMakeContextCurrent(winptr);

        if(!gladLoadGL())
        {
            glfwDestroyWindow(winptr);
            throw opres::FAILURE;
        }
    }
    void loop(render_callback fnc, const frameinfo& framedata, void* usrdata)
    {
        kill_loop.store(false);
        looping.store(true);
        while(!(glfwWindowShouldClose(winptr) || kill_loop.load()))
        {
            fnc(framedata, usrdata);

            glfwSwapBuffers(winptr);
            glfwPollEvents();
        }
        looping.store(false);
    }
    ~_impl()
    {
        glfwDestroyWindow(winptr);
    }
private:
    GLFWwindow* winptr;
};

aico::sys::wndctx::wndctx(int width, int height, const char* title, 
    renderer_t renderer) : renderfnc(renderer.fnc), stateptr(renderer.stateptr),
    implptr(new _impl(width, height, title)){}
aico::sys::wndctx::~wndctx()noexcept{delete implptr;}
aico::sys::wndctx::wndctx(wndctx&& other)noexcept : implptr(other.implptr){other.implptr = nullptr;}

void aico::sys::wndctx::interrupt() noexcept{implptr->kill_loop.store(true);}
void aico::sys::wndctx::loop()
{
    implptr->loop(renderfnc, _framedata, stateptr);
}
bool aico::sys::wndctx::looping()const noexcept{return implptr->looping.load();}
