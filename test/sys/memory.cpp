#include <cstdio>

#include "aico/sys/memory.h"


struct T{int x; float y;};
int main(int argc, char *argv[])
{
    auto a=aico::sys::malc<int>(10);
    auto b=aico::sys::malc<int>(10);
    auto c=aico::sys::malc<int>(10);

    aico::sys::fill_construct(a, 10, int(2));
    for(size_t i=0; i<10; ++i)
        printf("a[%lu]: %d\n", i, a[i]);
    
    aico::sys::default_construct(b, 10);
    for(size_t i=0; i<10; ++i)
        printf("b[%lu]: %d\n", i, b[i]);

    aico::sys::construct(c, 10, 3);
    for(size_t i=0; i<10; ++i)
        printf("c[%lu]: %d\n", i, c[i]);
    
    using namespace aico::sys;

    auto A=alloctr<int>(10);
    auto B=alloctr<int>(10);
    
    for(size_t i=0; i<10; ++i)
        printf("A[%lu]: %d\n", i, A[i]);

    for(size_t i=0; i<10; ++i)
        printf("B[%lu]: %d\n", i, B[i]);

    auto P=alloctr<T>(10, 1, 2.f);
    T foo{3, 4.f};
    auto Q=alloctr<T>(10, foo);
    auto S=alloctr<T>(10);

    for(size_t i=0; i<10; ++i)
        printf("P[%lu]: %d %f\n", i, P[i].x, P[i].y);

    for(size_t i=0; i<10; ++i)
        printf("Q[%lu]: %d %f\n", i, Q[i].x, Q[i].y);
    
    for(size_t i=0; i<10; ++i)
        printf("S[%lu]: %d %f\n", i, S[i].x, S[i].y);

    return 0;
}
