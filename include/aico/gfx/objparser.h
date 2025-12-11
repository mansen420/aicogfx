#pragma once

#include "aico/opres.h"
#include "aico/core/vec.h"
#include "aico/core/storage.h"


namespace aico
{
    struct vertex{vec3 pos, normal; vec2 uv;};

    storage<vertex> parseobj(const char* filename, opres_t* res);

    //TODO
    class obj;
}
