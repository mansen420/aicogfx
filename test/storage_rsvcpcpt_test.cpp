#include "opres.h"
#include "storage.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace aico;

struct Verbose {
    int value;
    static int alive;
    Verbose(int v = 0) : value(v) { ++alive; std::cout << "Verbose(" << value << ")\n"; }
    Verbose(const Verbose& o) : value(o.value) { ++alive; std::cout << "Copy(" << value << ")\n"; }
    Verbose(Verbose&& o) noexcept : value(o.value) { o.value = -1; ++alive; std::cout << "Move(" << value << ")\n"; }
    ~Verbose() { std::cout << "~Verbose(" << value << ")\n"; --alive; }
};
int Verbose::alive = 0;

struct ThrowMove {
    ThrowMove() = default;
    ThrowMove(ThrowMove&&) { std::cout << "ThrowMove move constructor called\n"; throw std::runtime_error("Move failed"); }
    ThrowMove(const ThrowMove&) = delete;
    ThrowMove& operator=(const ThrowMove&) = delete;
    ThrowMove& operator=(ThrowMove&&) = delete;
};

struct ThrowCopy {
    ThrowCopy() = default;
    ThrowCopy(const ThrowCopy&) { std::cout << "ThrowCopy copy constructor called\n"; throw std::runtime_error("Copy failed"); }
    ThrowCopy(ThrowCopy&&) = delete;
    ThrowCopy& operator=(const ThrowCopy&) = delete;
    ThrowCopy& operator=(ThrowCopy&&) = delete;
};

int main() {
    std::cout << "\n--- rsvcpct: Trivially Copyable ---\n";
    {
        storage<int> s(3, 42);
        assert(s.rsvcpct(3) == opres::SUCCESS); // no-op
        assert(s.rsvcpct(10) == opres::SUCCESS); // alloc + memcpy
    }

    std::cout << "\n--- rsvcpct: Verbose Logging ---\n";
    {
        Verbose::alive = 0;
        storage<Verbose> s(3, 7);
        assert(Verbose::alive == 3);
        s.rsvcpct(10);
        assert(Verbose::alive == 3);
    }

    std::cout << "\n--- rsvcpct: ThrowMove ---\n";
    {
        try {
            storage<ThrowMove> s(2);
            s.rsvcpct(10); // should throw
        } catch (const std::exception& e) {
            std::cout << "Caught exception: " << e.what() << "\nGood.\n";
        }
    }

    std::cout << "\n--- rsvcpct: ThrowCopy ---\n";
    {
        try {
            storage<ThrowCopy> s(2);
            s.rsvcpct(10); // should throw
        } catch (const std::exception& e) {
            std::cout << "Caught exception: " << e.what() << "\nGood.\n";
        }
    }

    std::cout << "\n--- rsvcpct: Absurd Reserve ---\n";
    {
        storage<int> s(2, 1);
        auto res = s.rsvcpct(1ull << 33); // likely fail
        assert(res == opres::MEM_ERR || res == opres::SUCCESS);
        if(res==opres::MEM_ERR)
            std::cout << "ALlocation fault, good.\n";
    }

    std::cout << "\nAll rsvcpct stress tests passed.\n";
}
