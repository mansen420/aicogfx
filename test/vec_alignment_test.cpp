#include "vector.h"
#include <iostream>

int main()
{
    using namespace aico;

    typedef vector<float, 2, true> vec2f;

    std::cout << sizeof(vec2f) << ", " << alignof(vec2f); //expected: 8, 8
}
