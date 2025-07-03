#include "storage.h"
#include <iostream>

int main()
{
    using namespace aico;
    typedef storage<float, 2> vec2f;
    
    vec2f x;
    x.at<0>() = 1.f;
    x.at<1>() = 2.f;

    for(auto f: x)
        std::cout << f;

    x[0]=3.f;
    x[1]=4.f;
    
    for(auto f: x)
        std::cout << f;
}
