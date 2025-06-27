#include "renderers.h"
#include "gfxctx.h"
#include "glad/glad.h"
#include "opres.h"
#include "wndctx.h"

float vertices[]
{
    0.f, 0.5f, -0.5f, 0.f, 0.5f, 0.f
};
const char* vtxsrc = 
    "#version 460 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}\n";
const char* frgsrc =
    "#version 460 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
        "FragColor = vec4(1.0); // white\n"
    "}\n";
using gfx = aico::gfxctx;
struct triangle_data
{
    gfx::shader_t *vtx = nullptr, *frg = nullptr;
    gfx::program_t* prog = nullptr;
    gfx::buf_t* vtxbuf = nullptr;
    gfx::vtxlayout_t* binding = nullptr;
    gfx* gpu;
    triangle_data(gfx* gpu)noexcept: gpu(gpu)
    {
        float vertices[]
        {
            0.f, 0.5f, -0.5f, 0.f, 0.5f, 0.f
        };

        vtxbuf = new gfx::buf_t(gpu->bufalloc({.size=sizeof(vertices), 
                .stride=2*sizeof(float)}, vertices));
        if(!vtxbuf)
            std::cout << "Uh oh..\n";

        using attrib = gfx::attribinfo;
        using bind = gfx::bindinfo;
        binding = new gfx::vtxlayout_t(gpu->make_vtxlayout(
                    {
                        .buffers{bind{*vtxbuf, 0, 0}},
                        .attribs{attrib{0, 2, 0, 0,
                            attrib::type::FLOAT}}
                    }));
        if(!binding)
            std::cout << "Uh oh..\n";
        gpu->bind(*binding);

        vtx = new gfx::shader_t(gpu->compile(gfx::stageinfo{vtxsrc, 
            gfx::stageinfo::type::VERT}));
        frg = new gfx::shader_t(gpu->compile(gfx::stageinfo{frgsrc,
            gfx::stageinfo::type::FRAG}));
        prog = new gfx::program_t(gpu->link({*frg, *vtx}));

        if(!(vtx && frg && prog))
            std::cout << "Uh oh..\n";
        gpu->bind(*prog);
        gpu->free(*vtx), gpu->free(*frg);
    }
    ~triangle_data()noexcept
    {
        gpu->free(*vtxbuf);
        gpu->free(*binding);
        gpu->free(*prog);
        delete binding;
        delete vtxbuf;
    }
};
aico::opres triangle_init(gfx* gpu, void*& state)
{
    state = new triangle_data(gpu);
    return aico::opres::SUCCESS;
}
void triangle_term(gfx*, void* state)
{
    auto data = (triangle_data*)(state);
    delete data;
}
void triangle_fn([[maybe_unused]] const aico::sys::wndctx::frameinfo&, gfx* gfxctxptr, 
    void* stateptr)
{
    auto state = (triangle_data*)(stateptr);
    
    glClear(GL_COLOR_BUFFER_BIT);

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
framedata, gfx*, void* stateptr)
{
    auto data = (flashing_red_data*)stateptr;
    
    const float factor = float(data->ctr)/data->max;
    glClearColor(factor * 1.f, 0.2f, 0.6f, 1.f);

    glClear(GL_COLOR_BUFFER_BIT);

    ++data->ctr %= data->max;
}
aico::sys::wndctx::renderer_t aico::sys::flashing_red{.fnc=flashing_red_fnc,
.stateptr = &data_g};
