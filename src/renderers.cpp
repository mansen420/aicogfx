#include "renderers.h"
#include "glad/glad.h"

struct flashing_red_data
{
    int ctr = 0;
    int max = 256;
};
flashing_red_data data_g;
void flashing_red_fnc([[maybe_unused]] const aicogfx::sys::wndctx::frameinfo&
    framedata, void* stateptr)
{
    flashing_red_data* data = (flashing_red_data*)stateptr;
    
    float factor = float(data->ctr)/data->max;
    glClearColor(factor * 1.f, 0.2f, 0.6f, 1.f);

    glClear(GL_COLOR_BUFFER_BIT);

    ++data->ctr %= data->max;
}
aicogfx::sys::renderer_t aicogfx::sys::flashing_red{.fnc=flashing_red_fnc,
    .stateptr = &data_g};
