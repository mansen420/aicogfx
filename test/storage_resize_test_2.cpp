#include "aico/storage.h"

#include <cassert>
#include <cstdio>
#include <iostream>

using namespace aico;

struct Verbose {
    int val;
    static inline int alive = 0;

    Verbose() : val(0) { ++alive; std::cout << "Default ctor\n"; }
    Verbose(int v) : val(v) { ++alive; std::cout << "Int ctor\n"; }
    Verbose(const Verbose& v) : val(v.val) { ++alive; std::cout << "Copy ctor\n"; }
    Verbose(Verbose&& v) noexcept : val(v.val) { ++alive; std::cout << "Move ctor\n"; }
    ~Verbose() { --alive; std::cout << "Dtor\n"; }
};

int main() {
    std::cout << "--- Default Resize Test ---\n";
    {
        storage<Verbose> s(2); // default ctor
        assert(s.size() == 2);
        assert(Verbose::alive == 2);

        s.resize(5); // grow with default
        assert(s.size() == 5);
        assert(Verbose::alive == 5);

        s.resize(3); // shrink
        assert(s.size() == 3);
        assert(Verbose::alive == 3);
    }
    assert(Verbose::alive == 0);

    std::cout << "--- Fill Resize Test ---\n";
    {
        Verbose filler(42);
        storage<Verbose> s(3, filler);
        assert(s.size() == 3);
        assert(Verbose::alive == 4); // 3 + 1 filler

        s.resize(6, filler); // fill
        assert(s.size() == 6);
        assert(Verbose::alive == 7); // 6 + 1 filler

        s.resize(2, filler); // shrink
        assert(s.size() == 2);
        assert(Verbose::alive == 3); // 2 + 1 filler
    }
    assert(Verbose::alive == 0);

    std::cout << "--- Custom Args Resize Test ---\n";
    {
        storage<Verbose> s(0);
        s.resize(4, 1337); // int arg constructor
        assert(s.size() == 4);
        for (size_t i = 0; i < 4; ++i)
            assert(s[i].val == 1337);
    }
    assert(Verbose::alive == 0);

    std::cout << "All resize tests passed.\n";
    return 0;
}
