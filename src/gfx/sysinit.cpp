#include "sysinit.h"
#include "_engctx.h"
#include "aico/gfx/engctx.h"

#include "GLFW/glfw3.h"

aico::opres_t aico::sys::init()
{
    if(!glfwInit())
        return FAIL;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    engflags = engflags | engctx::bits::INIT;

    return OK;
}

void aico::sys::terminate() noexcept
{
    glfwTerminate();
}
