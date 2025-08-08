#include "aico/malc.h" // your header, must be in include path / same dir
#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>
#include <random>
#include <set>

using namespace aico::sys;
using namespace aico;

#ifndef NDEBUG
#define DBG(x) do { x; } while(0)
#else
#define DBG(x)
#endif

bool is_aligned(void* p, size_t align) {
    return ((uintptr_t)p & (align - 1)) == 0;
}

void dump_header(void* userptr) {
    hdr_t* h = gethdr(userptr);
    if (!h) {
        std::cerr << "[!] invalid header for " << userptr << "\n";
        return;
    }
    std::cout << "  hdr @ " << h
              << " base=" << h->base
              << " bytes=" << h->bytes
              << " flags=" << (int)h->flags
              << " heapid=" << (int)h->heapid
              << "\n";
}

void test_alignment() {
    std::cout << "=== test_alignment ===\n";
    opres r;
    void* p = malc(100, 16, &r);
    assert(p);
    assert(r == opres::SUCCESS);
    assert(is_aligned(p, 16));
    std::cout << "Allocated 100 bytes at " << p << " aligned 16: OK\n";
    hdr_t* h = gethdr(p);
    assert(h && h->magic == 0xC0FFEE);
    dump_header(p);
    rel(p);
    std::cout << "Freed alignment test block\n\n";
}

void test_small_alloc() {
    std::cout << "=== test_small_alloc (size < freenode_t) ===\n";
    opres r;
    size_t request = 1; // smaller than freenode_t
    void* p = malc(request, alignof(hdr_t), &r);
    assert(p);
    assert(r == opres::SUCCESS);
    std::cout << "Requested " << request << " bytes, got user ptr " << p << "\n";
    hdr_t* h = gethdr(p);
    assert(h);
    // ensure underlying allocation was bumped to at least freenode_t
    assert(h->bytes >= sizeof(freenode_t));
    dump_header(p);
    rel(p);
    std::cout << "Freed small alloc\n\n";
}

void test_dedicated_alloc() {
    std::cout << "=== test_dedicated_alloc (largealloc) ===\n";
    opres r;
    // threshold in malc is 64*MB, so allocate something >= that
    void* p = malc(65 * MB, alignof(hdr_t), &r);
    assert(p);
    assert(r == opres::SUCCESS);
    hdr_t* h = gethdr(p);
    assert(h);
    assert(h->flags & hdr_t::DEDICATED);
    std::cout << "Large allocation went dedicated. Header:\n";
    dump_header(p);
    rel(p);
    std::cout << "Freed dedicated block\n\n";
}

void test_free_and_reuse() {
    std::cout << "=== test_free_and_reuse ===\n";
    opres r;
    void* a = malc(128, 8, &r);
    void* b = malc(128, 8, &r);
    assert(a && b);
    std::cout << "Allocated A=" << a << " B=" << b << "\n";

    // Write markers
    std::memset(a, 0xAA, 128);
    dump_header(a);
    std::memset(b, 0xBB, 128);
    dump_header(b);

    // Free A then B, freelist is LIFO so next alloc should return B first
    rel(a);
    rel(b);
    
    //use less bytes due to pessimistic fit logic
    void* c = malc(64, 8, &r);
    dump_header(c);
    assert(c == b && "Expected reuse of B (LIFO)"); 
    std::cout << "Reused B as expected: " << c << "\n";

    void* d = malc(64, 8, &r);
    assert(d == a && "Expected reuse of A second");
    std::cout << "Reused A as expected: " << d << "\n";

    rel(c);
    rel(d);
    std::cout << "Freed reused blocks\n\n";
}

void test_stress_random(size_t iterations = 10000) {
    std::cout << "=== test_stress_random ===\n";
    std::mt19937_64 rng(123456);
    std::uniform_int_distribution<size_t> size_dist(1, 1024);
    std::uniform_int_distribution<int> action(0, 1);
    struct AllocInfo { void* ptr; size_t size; };
    std::vector<AllocInfo> live;
    std::set<void*> seen;

    for (size_t i = 0; i < iterations; ++i) {
        if (!live.empty() && action(rng) == 0) {
            // free random
            size_t idx = rng() % live.size();
            rel(live[idx].ptr);
            live.erase(live.begin() + idx);
        } else {
            size_t sz = size_dist(rng);
            opres r;
            void* p = malc(sz, alignof(hdr_t), &r);
            if (!p) {
                std::cerr << "[!] allocation failed unexpectedly\n";
                continue;
            }
            assert(r == opres::SUCCESS);
            // check alignment
            assert(is_aligned(p, alignof(hdr_t)));
            hdr_t* h = gethdr(p);
            assert(h && h->magic == 0xC0FFEE);
            live.push_back({p, sz});
            seen.insert(p);
        }
    }
    // teardown
    for (auto& a : live)
        rel(a.ptr);
    std::cout << "Random stress test passed. Peak live allocations: " << live.size() << "\n\n";
}

int main() {
    std::cout << "=== malc/rel quick validation harness ===\n";
    test_alignment();
    test_small_alloc();
    test_dedicated_alloc();
    test_free_and_reuse();
    test_stress_random(5000);
    std::cout << "ALL TESTS PASSED (if no assert fired)\n";
    return 0;
}
