#include <cstddef>

namespace aico
{
    struct vec3 
    {
        union 
        {
            struct { float x, y, z; };
            struct { float r, g, b; };
            float data[3];
        };

        float& operator[](size_t i) { return data[i]; }
        const float& operator[](size_t i) const { return data[i]; }
    };
    struct vec2
    {
        union 
        {
            struct { float x, y; };
            struct { float r, g; };
            float data[2];
        };

        float& operator[](size_t i) { return data[i]; }
        const float& operator[](size_t i) const { return data[i]; }
    };
};
