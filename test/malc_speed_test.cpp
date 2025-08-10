#include "aico/malc.h"

#include <chrono>
#include <deque>
#include <vector>
#include <random>
#include <string>
#include <iostream>
#include <iomanip>

enum class Which { MALC, MALLOC };

struct Result 
{
    std::string test;
    std::string allocator;
    size_t operations;
    size_t total_bytes;
    double ms;
    double ns_per_op;
};

using Clock = std::chrono::steady_clock;

static void* do_alloc(Which w, size_t sz) 
{
    if (w == Which::MALLOC) return std::malloc(sz);
    aico::opres r;
    return aico::sys::malc(sz, alignof(std::max_align_t), &r);
}

static void do_free(Which w, void* p) 
{
    if (!p) return;
    if (w == Which::MALLOC) { std::free(p); return; }
    aico::sys::rel(p);
}

static Result test_bulk_alloc_free(Which w, size_t N, size_t sz) 
{
    std::vector<void*> ptrs;
    ptrs.reserve(N);
    auto t0 = Clock::now();
    for (size_t i=0; i<N; ++i) ptrs.push_back(do_alloc(w, sz));
    for (size_t i=0; i<N; ++i) do_free(w, ptrs[i]);
    auto t1 = Clock::now();
    double ms = std::chrono::duration<double,std::milli>(t1-t0).count();
    return {"bulk alloc+free size="+std::to_string(sz),
            (w==Which::MALLOC?"malloc":"malc"),
            N*2, N*sz, ms, (ms*1e6)/(N*2)};
}

static Result test_interleave_fifo(Which w, size_t N, size_t sz) 
{
    std::deque<void*> q;
    auto t0 = Clock::now();
    for (size_t i=0; i<N; ++i) {
        q.push_back(do_alloc(w, sz));
        if (i & 1) 
        { // free every other alloc
            do_free(w, q.front());
            q.pop_front();
        }
    }
    while (!q.empty()) { do_free(w, q.front()); q.pop_front(); }
    auto t1 = Clock::now();
    double ms = std::chrono::duration<double,std::milli>(t1-t0).count();
    return {"interleave FIFO size="+std::to_string(sz),
            (w==Which::MALLOC?"malloc":"malc"),
            N + (N/2) + (N/2), N*sz, ms, (ms*1e6)/(N+N)};
}

static Result test_random_sizes(Which w, size_t N, size_t min_sz, size_t max_sz) {
    std::mt19937_64 rng(12345);
    std::uniform_int_distribution<size_t> size_dist(min_sz, max_sz);
    std::vector<void*> live;
    live.reserve(N/2);
    size_t total_bytes = 0;
    auto t0 = Clock::now();
    for (size_t i=0; i<N; ++i) {
        bool alloc_op = (rng() & 1) || live.empty();
        if (alloc_op) {
            size_t sz = size_dist(rng);
            total_bytes += sz;
            live.push_back(do_alloc(w, sz));
        } else {
            size_t idx = rng() % live.size();
            do_free(w, live[idx]);
            live[idx] = live.back();
            live.pop_back();
        }
    }
    for (void* p : live) do_free(w, p);
    auto t1 = Clock::now();
    double ms = std::chrono::duration<double,std::milli>(t1-t0).count();
    return {"random sizes ["+std::to_string(min_sz)+","+std::to_string(max_sz)+"]",
            (w==Which::MALLOC?"malloc":"malc"),
            N, total_bytes, ms, (ms*1e6)/N};
}

int main() {
    const size_t N = 200; // adjust up for more stress
    const size_t randomN = 300;

    std::vector<size_t> sizes = {16, 32, 64, 128, 256, 512, 1024, 4096, 16384, 65536};
    std::vector<Result> results;

    for (auto sz : sizes) {
        // malloc
        results.push_back(test_bulk_alloc_free(Which::MALLOC, N, sz));
        results.push_back(test_interleave_fifo(Which::MALLOC, N, sz));
        // malc
        results.push_back(test_bulk_alloc_free(Which::MALC, N, sz));
        results.push_back(test_interleave_fifo(Which::MALC, N, sz));
    }

    // random sizes
    results.push_back(test_random_sizes(Which::MALLOC, randomN, 8, 1<<20));
    results.push_back(test_random_sizes(Which::MALC,   randomN, 8, 1<<20));

    // print
    std::cout << "test,allocator,operations,total_bytes,ms,ns_per_op\n";
    for (auto &r : results) {
        std::cout << r.test << ", " << r.allocator << ", "
                  << r.operations << ", " << r.total_bytes << ", "
                  << std::fixed << std::setprecision(3)
                  << r.ms << ", " << r.ns_per_op << "\n";
    }
}
