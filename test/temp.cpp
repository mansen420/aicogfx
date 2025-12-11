#include "aico/storage.h"

int main()
{
    using namespace aico;
    storage<int> container;
    container.rsvcpct(12000);
    
    for(size_t i=0; i<12000; ++i)
        container.push_back((int)i);
    return 0;
}
