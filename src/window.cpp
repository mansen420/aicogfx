#include "wndctx.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <atomic>
#include <cstddef>


uint32_t aicogfx::sys::engine_flags = 0;

aicogfx::sys::engctx::engctx()
{
    if(auto status = init(); status != opres::SUCCESS)
        throw status;
}
aicogfx::sys::engctx::~engctx()noexcept
{
    terminate();
}

aicogfx::sys::opres aicogfx::sys::init()
{
    if(!glfwInit())
        return opres::FAILURE;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    engine_flags = engine_flags | engine_flag_bits::INIT;

    return opres::SUCCESS;
}

void aicogfx::sys::terminate() noexcept
{
    glfwTerminate();
}


struct aicogfx::sys::wndctx::_impl
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
    int ctr = 0; //testing
    int max = 50;
    void loop()
    {
        kill_loop.store(false);
        looping.store(true);
        while(!(glfwWindowShouldClose(winptr) || kill_loop.load()))
        {
            glClear(GL_COLOR_BUFFER_BIT);

            float factor = float(ctr)/max;
            ++ctr %= max;
            glClearColor(factor * 1.0f, 0.2f, 0.5f, 1.f);

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

aicogfx::sys::wndctx::wndctx(int width, int height, const char* title) : implptr(new _impl(width, height, title)){}
aicogfx::sys::wndctx::~wndctx()noexcept{delete implptr;}
aicogfx::sys::wndctx::wndctx(wndctx&& other)noexcept : implptr(other.implptr){other.implptr = nullptr;}

void aicogfx::sys::wndctx::interrupt() noexcept{implptr->kill_loop.store(true);}
void aicogfx::sys::wndctx::loop(){implptr->loop();}
bool aicogfx::sys::wndctx::looping()const noexcept{return implptr->looping.load();}
