#pragma once

#include <cstdio>
#include <type_traits>
#include <utility>

#include "storage.h"

namespace aico
{
    template<typename T>
    void print(const T&, FILE*);

    inline void print(const char* obj, FILE* out)
    {
        fwrite(obj, 1, strlen(obj), out);
    }
    inline void print(int obj, FILE* out)
    {
    
    }
    inline void print(unsigned int obj, FILE* out)
    {
        fprintf(out, "%u", obj);
    }
    inline void print(float obj, FILE* out)
    {
        fprintf(out, "%f", obj);
    }

    template<typename T>
        concept Printable=requires(const T& val){::aico::print(val);};

    //maybe define this in `storage.h`? idk
    template<typename T>
        requires(Printable<T>)
    void print(const storage<T>&, FILE*);

    template<typename...Args>
        requires(Printable<Args>&&...)
    void fprint(const char* raw, Args...args);
    
    template<typename...Args>
        requires(Printable<Args>&&...)
    void fprint(const char* raw, FILE*, Args...args);
};
