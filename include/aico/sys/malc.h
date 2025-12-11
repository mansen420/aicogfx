#pragma once

#include "aico/sys/opres.h"

#include <cstddef>

namespace aico::sys
{
    void rel(void* usraddr)noexcept;
    void* malc(size_t bytes, size_t alignment=alignof(max_align_t), 
        opres_t* res=nullptr)noexcept;
    //HACKy
    template<typename T>
    size_t nr_objs(T* addr);
}
