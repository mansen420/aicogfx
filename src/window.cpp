#include "window.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"


uint32_t aicogfx::engine_flags = 0;

aicogfx::opres aicogfx::init()
{
    if(!glfwInit())
        return opres::FAILURE;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    engine_flags = engine_flags | engine_flag_bits::INIT;

    return opres::SUCCESS;
}

void aicogfx::terminate()
{
    glfwTerminate();
}

struct aicogfx::wndctx::_impl
{
    _impl(int width, int height, const char* title)
    {  
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
    ~_impl()
    {
        glfwDestroyWindow(winptr);
    }
    void loop()
    {
        while(!glfwWindowShouldClose(winptr))
        {
            glClear(GL_COLOR_BUFFER_BIT);

            glfwSwapBuffers(winptr);
            glfwPollEvents();
        }
    }
private:
    GLFWwindow* winptr;
};

aicogfx::wndctx::wndctx(int width, int height, const char* title) : implptr(new _impl(width, height, title)){implptr->loop();}
aicogfx::wndctx::~wndctx(){delete implptr;}
aicogfx::wndctx::wndctx(wndctx&& other) : implptr(other.implptr){other.implptr = nullptr;}
