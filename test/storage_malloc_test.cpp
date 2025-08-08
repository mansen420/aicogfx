#include "aico/storage.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <array>
#include <new>
#include <cassert>
#include <random>
#include <chrono>

using namespace aico;

// ---------- bind Alloc/Free to malloc/free ----------
static void* Malloc(size_t n) { return std::malloc(n); }
static void  Free(void* p)    { std::free(p); }

// shorthand alias: dynamic, no inline, Mincpct=8 by default, malloc/free
template<class T, size_t Mincpct = 8>
using storage_m = storage<T, DYNAMIC, false, Mincpct, &Malloc, &Free>;

// ---------- tiny timer (uses steady_clock) ----------
namespace aico 
{
    template<class D, class Clock = std::chrono::steady_clock>
    class timer {
    public:
        using duration = D; using time_point = typename Clock::time_point;
        timer(): _last(Clock::now()), _acc(duration::zero()) {}
        D tick(){auto n=Clock::now(); auto dt=std::chrono::duration_cast<D>(n-_last); _last=n; _acc+=dt; return dt;}
        D time_since_start() const {auto n=Clock::now(); return _acc+std::chrono::duration_cast<D>(n-_last);}
        void reset(){_acc=D::zero(); _last=Clock::now();}
    private: time_point _last; D _acc;
    };
    using micro_timer = timer<std::chrono::microseconds>;
}

// ---------- test types ----------
struct Pod32 { uint32_t a,b,c,d; };                 // trivial
struct HeavyPOD { std::array<uint64_t,16> v{}; };   // 128B trivial

struct NonTrivialDef 
{
    std::string s;
    NonTrivialDef() : s("xyz") {}
    NonTrivialDef(const char* p) : s(p) {}
};

struct NoDefault 
{
    int x;
    explicit NoDefault(int v) : x(v) {}
    NoDefault(const NoDefault&) = default;
    NoDefault(NoDefault&&)      = default;
    NoDefault& operator=(const NoDefault&) = default;
    NoDefault& operator=(NoDefault&&) = default;
};

// ---------- helpers ----------
template<class It>
static unsigned long long checksum(It first, It last) 
{
    unsigned long long h = 1469598103934665603ull;
    for (; first != last; ++first) {
        unsigned char buf[sizeof(*first)];
        std::memcpy(buf, std::addressof(*first), sizeof(*first));
        for (unsigned char b : buf) { h ^= b; h *= 1099511628211ull; }
    }
    return h;
}

static void line(const char* what, long long us, unsigned long long guard)
{
    std::printf("%-28s : %8lld us   (guard=%016llx)\n", what, us, guard);
}

// ---------- tests ----------

// A) trivial full-init copy path (memcpy) + container move
template<class T>
void test_trivial_copy_move(size_t N) 
{
    std::printf("\n=== %s :: trivial copy/move (N=%zu) ===\n", 
        typeid(T).name(), N);

    storage_m<T> s(N);// for trivial + default-constructible, 
                      // your ctor default-constructs; OK

    // initialize bytes deterministically (avoid UB from reading indeterminate)
    std::memset(s.begin(), 0xA5, N*sizeof(T));

    aico::micro_timer tm;

    // copy() -> should go down memcpy path and use full-init ctor (alivebits=null)
    tm.reset();
    auto cpy = s.template copy<T>();
    auto t_copy = tm.time_since_start().count();
    auto g1 = checksum(cpy.begin(), cpy.end());
    line("storage copy() [trivial]", t_copy, g1);

    // container move (O(1))
    tm.reset();
    auto moved = std::move(cpy);
    auto t_mvc = tm.time_since_start().count();
    auto g2 = checksum(moved.begin(), moved.end());
    line("storage container move", t_mvc, g2);
}

// B) partial-init ctor (external bits) -> rsvcpct() growth -> dtor
// This tries to catch mask copy/embedding issues.
template<class T>
void test_partial_init_and_reserve(size_t size, size_t grow_to, size_t bit_offset = 0) 
{
    std::printf("\n=== %s :: partial-init + rsvcpct ===\n", typeid(T).name());

    // raw uninitialized buffer for T
    T* raw = static_cast<T*>(std::malloc(std::max(size, 
        (size_t)8) * sizeof(T)));
    assert(raw && "malloc failed");

    // craft a fake bit mask with alternating 101010... pattern
    const size_t src_bits = size + bit_offset;
    const size_t src_nbytes = (src_bits + 7) / 8;
    std::vector<uint8_t> bits(std::max<size_t>(1, src_nbytes), 0);
    for (size_t i = 0; i < size; ++i) {
        const size_t b = (bit_offset + i) / 8, off = (bit_offset + i) % 8;
        if (i % 2 == 0) bits[b] |= uint8_t(1u << off);
    }

    // construct storage from raw + bits 
    // (this allocates a separate _alivebits internally)
    storage_m<T> s(raw, size, bits.data(), bit_offset);

    aico::micro_timer tm;

    // grow capacity (should co-allocate embedded mask and free the old one)
    tm.reset();
    auto r = s.rsvcpct(grow_to);
    auto t_res = tm.time_since_start().count();
    line("rsvcpct(partial-init)", t_res, (unsigned long long)r);

    // shrink/expand size to exercise _destroy paths
    s.resize(size / 2);
    s.resize(size);
}

// C) non-default-constructible path (tracks bits), push via resize(fill)
void test_no_default_push_resize(size_t N) {
    std::printf("\n=== NoDefault :: push/resize (N=%zu) ===\n", N);
    storage_m<NoDefault> s(0); // ctor will track bits (uninitialized path)
    aico::micro_timer tm;

    tm.reset();
    s.rsvcpct(N);
    for (size_t i = 0; i < N; ++i) 
    {
        s.resize(s.size() + 1, NoDefault(int(i))); // your current push_back calls this anyway
    }
    auto t_push = tm.time_since_start().count();
    auto g1 = checksum(s.begin(), s.end());
    line("resize(+1, fill)", t_push, g1);

    tm.reset();
    s.resize(N * 2, NoDefault(42));
    auto t_resz = tm.time_since_start().count();
    auto g2 = checksum(s.begin(), s.end());
    line("resize(N*2, fill)", t_resz, g2);

    // element-wise move into a new storage (O(n))
    tm.reset();
    auto moved = s.template move<NoDefault>();
    auto t_mve = tm.time_since_start().count();
    auto g3 = checksum(moved.begin(), moved.end());
    line("move() elem-wise", t_mve, g3);
}

// D) non-trivial type (std::string) basic ops
void test_nontrivial_string(size_t N) 
{
    std::printf("\n=== NonTrivialDef(std::string) (N=%zu) ===\n", N);
    storage_m<NonTrivialDef> s(0);
    aico::micro_timer tm;

    tm.reset();//TODO XXX malloc crashes when we call rsvcpct() here!
    s.rsvcpct(N);
    for (size_t i = 0; i < N; ++i) 
        s.resize(s.size()+1, NonTrivialDef("abc"));
    auto t_push = tm.time_since_start().count();
    auto g1 = checksum(s.begin(), s.end());
    line("push via resize(fill)", t_push, g1);

    tm.reset();
    auto cpy = s.template copy<NonTrivialDef>();
    auto t_copy = tm.time_since_start().count();
    auto g2 = checksum(cpy.begin(), cpy.end());
    line("copy()", t_copy, g2);

    tm.reset();
    auto mv = s.template move<NonTrivialDef>();
    auto t_move = tm.time_since_start().count();
    auto g3 = checksum(mv.begin(), mv.end());
    line("move()", t_move, g3);
}

// E) brute test: many copy()/rsvcpct() cycles on HeavyPOD
void test_heavypod_cycles(size_t N, int rounds) 
{
    std::printf("\n=== HeavyPOD cycles (N=%zu, rounds=%d) ===\n", N, rounds);
    storage_m<HeavyPOD> s(N);
    std::memset(s.begin(), 0xCC, N*sizeof(HeavyPOD));

    for (int r = 0; r < rounds; ++r) 
    {
        auto c = s.template copy<HeavyPOD>(N, 0);
        s.rsvcpct(N + (r+1)*8);
        s.resize((r%2) ? N/2 : N);
        auto g = checksum(c.begin(), c.end());
        // keep the compiler honest
        if (g == 0xDEADBEEF) std::puts("lol");
    }
    std::puts("cycles done");
}

int main() {
    // A: trivial fast path sanity
    test_trivial_copy_move<Pod32>(200000);
    test_trivial_copy_move<HeavyPOD>(200000);

    // B: partial-init + reserve grow (tries to catch mask embed/copy bugs)
    test_partial_init_and_reserve<HeavyPOD>(/*size*/1024, /*grow_to*/4096, /*bit_offset*/3);
    test_partial_init_and_reserve<Pod32>(/*size*/4096, /*grow_to*/8192, /*bit_offset*/7);

    // C: non-default-constructible flow (tracks bits)
    test_no_default_push_resize(50000);

    // D: non-trivial type (std::string)
    test_nontrivial_string(20000);

    // E: stress cycles on HeavyPOD
    test_heavypod_cycles(8192, 64);

    std::puts("\nOK.");
    return 0;
}
