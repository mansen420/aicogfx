#include "storage.h"
#include <cassert>
#include <iostream>


struct Verbose {
    int value;
    static inline int alive = 0;

    Verbose(int v) : value(v) {++alive; std::cout << "Construct " << value << "\n"; }
    Verbose(const Verbose& other) : value(other.value) {
        std::cout << "Copy " << value << "\n";
    }
    Verbose(Verbose&& other) noexcept : value(other.value) {
        ++alive;
        std::cout << "Move " << value << "\n";
        other.value = -1;
    }
    Verbose& operator=(const Verbose& other) {
        std::cout << "Copy Assign " << other.value << " -> " << value << "\n";
        value = other.value;
        return *this;
    }
    Verbose& operator=(Verbose&& other) noexcept {
        std::cout << "Move Assign " << other.value << " -> " << value << "\n";
        value = other.value;
        other.value = -1;
        return *this;
    }
    ~Verbose() {
        --alive;
        std::cout << "Destroy " << value << "\n";
    }
};

using namespace aico;
int main() {
    using VStore = storage<Verbose, DYNAMIC, false, 4, malloc, free>;

    std::cout << "--- Initialized destination ---\n";
    {
        VStore src(5, 100); // Verbose(100)
        assert(Verbose::alive == 5);

        VStore dst(5, 999); // pre-init with Verbose(999)
        assert(Verbose::alive == 10);

        auto res = src.move(3, dst, 1, 1, false); // overwrite dst[1..3]
        assert(res == opres::SUCCESS);
        assert(dst[1].value == 100);
        assert(dst[2].value == 100);
        assert(dst[3].value == 100);

        std::cout << "--- Done with move (non-initialize) ---\n";
    }
    assert(Verbose::alive == 0);

    std::cout << "\n--- Uninitialized destination ---\n";
    {
        VStore src(3, 42);
        assert(Verbose::alive == 3);

        Verbose* raw = (Verbose*)malloc(4 * sizeof(Verbose));
        VStore dst(raw, 4, false); // uninitialized ownership

        auto res = src.move(3, dst, 0, 0, true); // initialize in-place
        assert(res == opres::SUCCESS);
        assert(dst[0].value == 42);
        assert(dst[1].value == 42);
        assert(dst[2].value == 42);

        std::cout << "--- Done with move (initialize) ---\n";
    }
    assert(Verbose::alive == 0);

    std::cout << "\nAll move segment tests passed.\n";
    return 0;
}
