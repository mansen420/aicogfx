#include "renderers.h"
#include "gfxctx.h"
#include "glad/glad.h"
#include "opres.h"
#include "wndctx.h"
#include "_gfxctx.h"

float vertices[]
{
    0.f, 0.5f, -0.5f, 0.f, 0.5f, 0.f
};
const char* vtxshd = 
    "#version 460 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}\n";
const char* const frgshd =
    "#version 460 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
        "FragColor = vec4(1.0); // white\n"
    "}\n";
struct triangle_data
{
    GLuint VAO, EBO, VTX, FRG, PROG;
    aico::gfxctx::buf_t* vtxbuf;
    aico::gfxctx* gpu;
    triangle_data(aico::gfxctx* gpu)noexcept : gpu(gpu)
    {
        glCreateVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        float vertices[]
        {
            0.f, 0.5f, -0.5f, 0.f, 0.5f, 0.f
        };

        vtxbuf = new aico::gfxctx::buf_t(gpu->bufalloc({.size=sizeof(vertices), 
                .stride=2*sizeof(float)}, vertices));
        if(!vtxbuf)
            std::cout << "Uh oh..\n";
        
        glVertexArrayVertexBuffer(VAO, 0, 
            gpu->getimpl()->gethndl(*vtxbuf), 0, 
                2 * sizeof(float));
        glEnableVertexArrayAttrib(VAO, 0);
        glVertexArrayAttribFormat(VAO, 0, 2, GL_FLOAT,
            GL_FALSE, 0);
        glVertexArrayAttribBinding(VAO, 0, 0);

        VTX = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(VTX, 1, &vtxshd, nullptr);
        glCompileShader(VTX);
        GLint status;
        glGetShaderiv(VTX, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE)
            std::cout << "dude...\n";

        FRG = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(FRG, 1, &frgshd, nullptr);
        glCompileShader(FRG);
        glGetShaderiv(FRG, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE)
            std::cout << "dude...\n";
        PROG = glCreateProgram();
        glAttachShader(PROG, VTX);
        glAttachShader(PROG, FRG);
        glLinkProgram(PROG);
        
        glGetProgramiv(PROG, GL_LINK_STATUS, &status);
        if(status == GL_FALSE)
            std::cout << "dude...\n";

        glDeleteShader(FRG); glDeleteShader(VTX);
    }
    ~triangle_data()noexcept
    {
        glDeleteProgram(PROG);
        gpu->freebuf(*vtxbuf);
        delete vtxbuf;
        glDeleteVertexArrays(1, &VAO);
    }
};
aico::opres triangle_init(aico::gfxctx* gpu, void*& state)
{
    state = new triangle_data(gpu);
    return aico::opres::SUCCESS;
}
void triangle_term(aico::gfxctx*, void* state)
{
    auto data = (triangle_data*)(state);
    delete data;
}
void triangle_fn([[maybe_unused]] const aico::sys::wndctx::frameinfo&,
    aico::gfxctx* gfxctxptr, void* stateptr)
{
    auto gfx = gfxctxptr->getimpl();
    auto state = (triangle_data*)(stateptr);
    
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(state->PROG);
    glBindVertexArray(state->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

aico::sys::wndctx::renderer_t aico::sys::triangle=
{
    .fnc = triangle_fn, .initfnc = triangle_init, .termfnc=triangle_term
};

struct flashing_red_data
{
    int ctr = 0;
    int max = 256;
};
flashing_red_data data_g;
void flashing_red_fnc([[maybe_unused]] const aico::sys::wndctx::frameinfo&
framedata, aico::gfxctx*, void* stateptr)
{
    auto data = (flashing_red_data*)stateptr;
    
    const float factor = float(data->ctr)/data->max;
    glClearColor(factor * 1.f, 0.2f, 0.6f, 1.f);

    glClear(GL_COLOR_BUFFER_BIT);

    ++data->ctr %= data->max;
}
aico::sys::wndctx::renderer_t aico::sys::flashing_red{.fnc=flashing_red_fnc,
.stateptr = &data_g};
