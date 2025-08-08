#include "aico/malc.h"
#include <iostream>
#include <cassert>

using namespace aico::sys;
using namespace aico;

void test_heap_growth()
{
    std::cout << "== Heap Growth Test ==\n";
    std::cout << "Initial g_heapsz = " << g_heapsz << "\n";
    size_t alloc_size = 512 * KB;
    size_t target_allocs = 100;
    opres res;

    size_t last_heapsz = 0;
    size_t growth_count = 0;

    for (size_t i = 0; i < target_allocs; ++i)
    {
        void* ptr = malc(alloc_size, alignof(max_align_t), &res);
        assert(ptr != nullptr);
        assert(res == opres::SUCCESS);
        
        if (g_heapsz > last_heapsz)
        {
            std::cout << "Heap grew to " << (int)g_heapsz << " after " << i+1
                      << " allocations\n";
            last_heapsz = g_heapsz;
            growth_count++;
        }

        alloc_size += 128 * KB;
    }

    std::cout << "\n== Final Heap States ==\n";
    for (size_t i = 0; i < g_heapsz; ++i)
    {
        std::cout << "Heap #" << i
                  << " size: " << g_heaps[i].bytes / KB << " KB"
                  << " | offset: " << g_heaps[i].offset / KB << " KB\n";
    }

    std::cout << "\nTotal heaps created: " << g_heapsz << "\n";
    std::cout << "Total growth events: " << growth_count << "\n";

    assert(g_heapsz > 1);
    std::cout << "Heap growth test passed ✅\n";
}

int main()
{
    test_heap_growth();
    return 0;
}
