#include "aico/malc.h" 
#include <iostream>
#include <cassert>

using namespace aico::sys;
using namespace aico;

void test_basic_allocation()
{
    std::cout << "Test: Basic Allocation\n";
    opres res;
    void* ptr = malc(1024, alignof(max_align_t), &res);
    assert(ptr != nullptr);
    assert(res == opres::SUCCESS);

    hdr_t* header = header_addr(ptr);
    assert(header->magic == 0xC0FFEE);
    assert(header->bytes >= 1024);
    assert(header->base != nullptr);

    std::cout << "Basic Allocation passed ✅\n";
}

void test_alignment()
{
    std::cout << "Test: Alignment\n";
    for (size_t align = 8; align <= 256; align *= 2)
    {
        opres res;
        void* ptr = malc(128, align, &res);
        assert(ptr != nullptr);
        assert(res == opres::SUCCESS);
        assert(reinterpret_cast<uintptr_t>(ptr) % align == 0);
    }
    std::cout << "Alignment test passed ✅\n";
}

void test_invalid_alignment()
{
    std::cout << "Test: Invalid Alignment\n";
    opres res;
    void* ptr = malc(128, 3, &res); // not a power of two
    assert(ptr == nullptr);
    assert(res == opres::ALIGN_ERR);

    ptr = malc(128, 1, &res); // < alignof(void*)
    assert(ptr == nullptr);
    assert(res == opres::ALIGN_ERR);

    std::cout << "Invalid Alignment test passed ✅\n";
}

void test_large_allocation()
{
    std::cout << "Test: Large Allocation (Dedicated)\n";
    opres res;
    void* ptr = malc(100 * MB, alignof(max_align_t), &res); // triggers dedicated
    assert(ptr != nullptr);
    assert(res == opres::SUCCESS);

    hdr_t* header = header_addr(ptr);
    assert(header->magic == 0xC0FFEE);
    assert(header->flags & hdr_t::DEDICATED);

    std::cout << "Large Allocation passed ✅\n";
}

int main()
{
    test_basic_allocation();
    test_alignment();
    test_invalid_alignment();
    test_large_allocation();
    std::cout << "All tests passed 🎉\n";
    return 0;
}
