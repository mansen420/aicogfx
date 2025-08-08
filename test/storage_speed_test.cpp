#include "aico/opres.h"
#include "aico/storage.h"
#include "aico/timer.h"

#include <type_traits>
#include <vector>
#include <string>
#include <array>
#include <memory>
#include <random>
#include <cstdio>
#include <cstdint>
#include <cassert>

// ================== test types ==================
struct Pod32 
{
    uint32_t a, b, c, d; // trivially copyable, default-constructible
};

struct HeavyPOD 
{
    std::array<uint64_t, 16> v{}; // 128 bytes, trivially copyable
};

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

struct MoveOnly 
{
    std::unique_ptr<int> p;
    explicit MoveOnly(int v) : p(std::make_unique<int>(v)) {}
    MoveOnly(MoveOnly&&) noexcept = default;
    MoveOnly& operator=(MoveOnly&&) noexcept = default;
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
};

// ============ helpers to defeat dead-code elimination ============
template<class It>
static uint64_t checksum(It first, It last) 
{
    // Very cheap accumulation to keep the compiler from throwing things away
    uint64_t h = 1469598103934665603ull; // FNV offset
    for (; first != last; ++first) 
    {
        // interpret bytes without UB; use memcpy
        unsigned char buf[sizeof(*first)];
        std::memcpy(buf, std::addressof(*first), sizeof(*first));
        for (unsigned char b : buf) 
        {
            h ^= b;
            h *= 1099511628211ull;
        }
    }
    return h;
}

static void print_t(const char* label, long long us, uint64_t guard) 
{
    std::printf("%-28s : %8lld us   (guard=%016llx)\n", label, us, 
        (unsigned long long)guard);
}

// ================== benchmarks for std::vector ===================
template<class T>
static void bench_vector(std::size_t N, std::mt19937_64& rng) 
{
    using namespace std;
    using aico::micro_timer;

    vector<T> v;
    v.reserve(N);

    micro_timer tm;

    // reserve + push_back
    tm.reset();
    for (size_t i = 0; i < N; ++i) 
    {
        if constexpr (std::is_same_v<T, NoDefault> || std::is_same_v<T, MoveOnly>)
            v.push_back(T(int(i)));
        else if constexpr (std::is_same_v<T, NonTrivialDef>)
            v.push_back(T("abc"));
        else
            v.push_back(T{});
    }
    auto us_push = tm.time_since_start().count();
    auto g1 = checksum(v.begin(), v.end());
    print_t("vector push_back", us_push, g1);

    // copy construct
    if constexpr(!std::is_same_v<T, MoveOnly>)
    {
        tm.reset();
        vector<T> vcopy = v;
        auto us_copy = tm.time_since_start().count();
        auto g2 = checksum(vcopy.begin(), vcopy.end());
        print_t("vector copy", us_copy, g2);
    }

    // move construct
    tm.reset();
    vector<T> vmove = std::move(v);
    auto us_move = tm.time_since_start().count();
    auto g3 = checksum(vmove.begin(), vmove.end());
    print_t("vector move", us_move, g3);

    // resize larger (default / fill / args)
    tm.reset();
    if constexpr (std::is_default_constructible_v<T>)
        vmove.resize(N * 2);
    else if constexpr (std::is_same_v<T, NoDefault>)
        vmove.resize(N * 2, T(42));
    else if constexpr (std::is_same_v<T, MoveOnly>) 
    {
        vmove.reserve(N * 2);
        for (size_t i = vmove.size(); i < N * 2; ++i) vmove.push_back(T(7));
    }
    auto us_resize = tm.time_since_start().count();
    auto g4 = checksum(vmove.begin(), vmove.end());
    print_t("vector resize(+)", us_resize, g4);

    // random read
    std::uniform_int_distribution<size_t> dist(0, vmove.size() ? vmove.size()-1 : 0);
    tm.reset();
    uint64_t guard = 0;
    for (size_t i = 0; i < N; ++i)
        guard ^= dist(rng);

    auto us_rand = tm.time_since_start().count();
    print_t("vector rand idx (dummy)", us_rand, guard);
}

// ============ benchmarks for aico::storage =======================

template<class T>
static void bench_storage(std::size_t N, std::mt19937_64& rng) 
{
    using namespace std;
    using aico::micro_timer;
    
    // assumes default-constructible container itself
    aico::storage<T> s;
    s.rsvcpct(N);

    micro_timer tm;

    // reserve + push_back
    tm.reset();
    for (size_t i = 0; i < N; ++i) {
        if constexpr (std::is_same_v<T, NoDefault> || std::is_same_v<T, MoveOnly>)
            s.push_back(T(int(i)));
        else if constexpr (std::is_same_v<T, NonTrivialDef>)
            s.push_back(T("abc"));
        else
            s.push_back(T{});
    }
    auto us_push = tm.time_since_start().count();
    auto g1 = checksum(s.begin(), s.end());
    print_t("storage push_back", us_push, g1);

    if constexpr(!std::is_same_v<T, MoveOnly>)
    {
        // copy (your .copy() returns by value)
        tm.reset();
        auto scopy = s.template copy<T>();
        auto us_copy = tm.time_since_start().count();
        auto g2 = checksum(scopy.begin(), scopy.end());
        print_t("storage copy()", us_copy, g2);
    }

    // move (your .move() returns by value)
    tm.reset();
    auto smove = s.template move<T>();
    auto us_move = tm.time_since_start().count();
    auto g3 = checksum(smove.begin(), smove.end());
    print_t("storage move()", us_move, g3);

    // resize larger (default / fill / args)
    tm.reset();
    if constexpr (std::is_default_constructible_v<T>)
        s.resize(N * 2);
    else if constexpr (std::is_same_v<T, NoDefault>)
        s.resize(N * 2, T(42));
    else if constexpr (std::is_same_v<T, MoveOnly>) 
    {
        s.rsvcpct(N * 2);
        for (size_t i = s.size(); i < N * 2; ++i) s.push_back(T(7));
    }
    auto us_resize = tm.time_since_start().count();
    auto g4 = checksum(s.begin(), s.end());
    print_t("storage resize(+)", us_resize, g4);

    // random read (dummy)
    std::uniform_int_distribution<size_t> dist(0, s.size() ? s.size()-1 : 0);
    tm.reset();
    uint64_t guard = 0;
    for (size_t i = 0; i < N; ++i) 
        guard ^= dist(rng);

    auto us_rand = tm.time_since_start().count();
    print_t("storage rand idx (dummy)", us_rand, guard);
}

// ================== driver ======================
template<class T>
static void run_case(const char* name, std::size_t N, std::mt19937_64& rng) 
{
    std::printf("\n=== %s (N=%zu) ===\n", name, N);
    bench_vector<T>(N, rng);
    bench_storage<T>(N, rng);
}

int main() {
    std::mt19937_64 rng(12345);

    const std::size_t N = 200000; // tweak for your box

    run_case<Pod32>("Pod32 (trivial)", N, rng);
    run_case<HeavyPOD>("HeavyPOD (128B trivial)", N, rng);
    // slower; fewer elems
    run_case<NonTrivialDef>("NonTrivialDef (std::string)", N/10, rng); 
    run_case<NoDefault>("NoDefault (no default ctor)", N, rng);
    run_case<MoveOnly>("MoveOnly (unique_ptr)", N/5, rng);

    return 0;
}

