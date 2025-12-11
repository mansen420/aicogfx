#pragma once

#include <cstdio>

namespace aico::sys
{
    template<typename T>
    void print(FILE*, const T&)noexcept;

    template<typename...Args>
    void printf(const char*, const Args&...)noexcept;

    template<typename...Args>
    void fprintf(const char*, FILE*, const Args&...)noexcept;
};
