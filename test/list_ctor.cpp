#include "list.h"
#include "timer.h"
#include <algorithm>
#include <cstddef>
#include <cstdio>

int main()
{
    aico::micro_timer time;
    
    typedef list<char, 4, 1> str4;
    str4 ABCD = {'A', 'B', 'C', 'D'};

    list<str4> buffer(size_t(4096 * 1000));

    std::fill(buffer.begin(), buffer.end(), ABCD);
    
    auto interval = time.time_since_start();
    printf("%li micro seconds.\n", interval.count());

    buffer.print(std::cout);
    return 0;
}
