#include <cassert>
#include <iostream>
#include <stdexcept>
#include "storage.h" 

using namespace aico;

// a non-trivial type that can throw on copy
struct Exploder {
    int v;
    static bool explode;
    Exploder(int x): v(x) {}
    Exploder(const Exploder& o) {
        if (explode) throw std::runtime_error("copy bomb");
        v = o.v;
    }
    Exploder(Exploder&& o) noexcept : v(o.v) { o.v = -1; }
    ~Exploder() {}
};
bool Exploder::explode = false;

// simple POD for fast path checks
struct POD { int x, y; };

void test_static_pod() {
    storage<POD, 4> s;      // static dim
    assert(s.size() == 4);
    for (size_t i = 0; i < s.size(); ++i) {
        s[i].x = int(i);
        s[i].y = int(i*2);
    }
    for (size_t i = 0; i < s.size(); ++i) {
        assert(s[i].x == int(i));
        assert(s[i].y == int(i*2));
    }
    std::cout << "✔ static POD OK\n";
}

void test_dynamic_default_constructible() {
    storage<int> s(10);    // T() exists, no bit-tracking
    assert(s.size() == 10);
    for (size_t i = 0; i < s.size(); ++i) s[i] = int(i*3);
    auto s2 = s.copy<int>();
    assert(s2.size() == s.size());
    for (size_t i = 0; i < s.size(); ++i) assert(s2[i] == s[i]);
    std::cout << "✔ dynamic int copy OK\n";
}

void test_dynamic_non_default() {
    // Exploder has no T(), force trackbits
    storage<Exploder> s(5, /*fillval*/ Exploder(7));
    assert(s.size() == 5);
    for (size_t i = 0; i < s.size(); ++i) 
        assert(s[i].v == 7);

    // test construct_at & bit-tracking
    Exploder::explode = false;
    storage<Exploder> s2(3);               // uses non-default ctor path
    // before construct all bits must be false
    for (size_t i = 0; i < s2.size(); ++i) {
        // _alivebits pointer non-null, so alive(i)==false
        assert(!s2._alive(i));
    }
    s2.construct_at(1, 42);
    assert(s2[1].v == 42 && s2._alive(1));
    std::cout << "✔ non-default & construct_at OK\n";
}

void test_reserve_and_resize() {
    storage<int> s(4);
    for (int i = 0; i < 4; ++i) s[i] = i;
    auto oldcap = s.rsvcpct(8);
    assert(oldcap == opres::SUCCESS);
    assert(s.size() == 4);                  // size unchanged
    // test resize up
    auto r1 = s.resize(6, 99);
    assert(r1 == opres::SUCCESS);
    assert(s.size() == 6);
    for (size_t i = 4; i < s.size(); ++i) assert(s[i] == 99);
    // test resize down
    auto r2 = s.resize(2);
    assert(r2 == opres::SUCCESS);
    assert(s.size() == 2);
    std::cout << "✔ reserve/resize OK\n";
}

void test_move_semantics() {
    storage<int> s(5);
    for (int i = 0; i < 5; ++i) s[i] = i*5;
    auto moved = std::move(s);
    assert(moved.size() == 5);
    for (int i = 0; i < 5; ++i) assert(moved[i] == i*5);
    // old s is now zeroed
    assert(s.size() == 0);
    std::cout << "✔ move ctor OK\n";
}

void test_exception_safety() {
    // make copy bomb
    storage<Exploder> src(3, Exploder(5));
    Exploder::explode = true;
    try {
        auto bad = src.copy<Exploder>();
        assert(false && "copy should have thrown");
    } catch (const std::runtime_error&) {
        // original unscathed
        assert(src.size() == 3);
        for (size_t i = 0; i < src.size(); ++i) assert(src[i].v == 5);
        std::cout << "✔ exception safety on copy OK\n";
    }
    Exploder::explode = false;
}

void test_alignment() {
    storage<double> s(7);
    auto ptr = reinterpret_cast<uintptr_t>(s.begin());
    assert(ptr % alignof(double) == 0 && "misaligned data");
    std::cout << "✔ alignment OK\n";
}

int main() {
    try {
        test_static_pod();
        test_dynamic_default_constructible();
        test_dynamic_non_default();
        test_reserve_and_resize();
        test_move_semantics();
        test_exception_safety();
        test_alignment();
    } catch (const std::exception& e) {
        std::cerr << "💥 TEST FAILURE: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "💥 UNKNOWN FAILURE\n";
        return 1;
    }
    std::cout << "ALL TESTS PASSED—storage holds up…for now.\n";
    return 0;
}
