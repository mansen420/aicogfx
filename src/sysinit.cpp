#include "sysinit.h"
#include "_engctx.h"
#include "engctx.h"

#include "GLFW/glfw3.h"

aico::opres aico::sys::init()
{
    if(!glfwInit())
        return opres::FAILURE;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    engflags = engflags | engctx::bits::INIT;

    return opres::SUCCESS;
}

void aico::sys::terminate() noexcept
{
    glfwTerminate();
}
