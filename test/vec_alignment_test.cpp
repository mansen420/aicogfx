#include "storage.h"
#include <iostream>

int main()
{
    using namespace aico;

    typedef storage<float, 2> vec2f;

    std::cout << sizeof(vec2f) << ", " << alignof(vec2f); //expected: 8, 8
}
