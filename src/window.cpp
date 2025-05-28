#include "window.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

GLFWwindow* WND;
int aicogfx::create_window(int width, int height, char* title)
{
    if(!glfwInit())
        return STATUS_FAILURE;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    WND = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if(!WND)
        return STATUS_FAILURE;
    
    glfwMakeContextCurrent(WND); //make OpenGL context current FIRST
    
    if(gladLoadGL() == 0) // THEN load OpenGL functions!
        return STATUS_FAILURE;

    return STATUS_SUCCESS;
}

bool aicogfx::window_should_close()
{
    return glfwWindowShouldClose(WND);
}

void aicogfx::process_shit()
{
    glClearColor(1.f, 0.2f, 0.3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(WND);
    glfwPollEvents();
}
void aicogfx::destroy_window()
{
    glfwDestroyWindow(WND);
    glfwTerminate();
}
