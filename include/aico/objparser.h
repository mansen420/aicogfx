#pragma once

#include "opres.h"
#include "vec.h"
#include "storage.h"


namespace aico
{
    struct vertex{vec3 pos, normal; vec2 uv;};

    storage<vertex> parseobj(const char* filename, opres* res);

    //TODO
    class obj;
}
