#include "list.h"
#include "timer.h"
#include <cstddef>
#include <cstdio>

int main()
{
    aico::micro_timer time;

    list<char> buffer(size_t(4096 * 1000)); 

    buffer.mutate([](char& byte, [[maybe_unused]] size_t idx){byte = 0;});
    
    auto interval = time.clock();
    printf("%li micro seconds.\n", interval.count());
    return 0;
}
