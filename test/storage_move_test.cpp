#include "aico/storage.h"

#include <iostream>
#include <cassert>
#include <cstdlib>

// A verbose type to track lifecycle
struct Verbose {
    int value = -999;
    static inline int alive = 0;

    Verbose(int v) : value(v) { ++alive; std::cout << "Construct " << value << '\n'; }
    Verbose(const Verbose& v) : value(v.value) { ++alive; std::cout << "Copy " << value << '\n'; }
    Verbose(Verbose&& v) noexcept : value(v.value) {
        ++alive;
        v.value = -1;
        std::cout << "Move " << value << '\n';
    }
    ~Verbose() { std::cout << "Destroy " << value << '\n'; --alive; }
};

// Test allocator functions
void* raw_alloc(size_t n) {
    std::cout << "[alloc " << n << " bytes]\n";
    return std::malloc(n);
}
void raw_free(void* p) {
    std::cout << "[free]\n";
    std::free(p);
}

using namespace aico;
int main() {
    using SourceStore = storage<Verbose>;
    using DestStore = storage<Verbose, DYNAMIC, false, 8, raw_alloc, raw_free>;

    // Create a source storage and fill with distinct values
    SourceStore a(5);
    for (size_t i = 0; i < a.size(); ++i)
        std::construct_at(a.begin() + i, int(i * 10)); // [0, 10, 20, 30, 40]

    assert(Verbose::alive == 5);

    // Move 3 elements starting from index 1 into a new container
    opres result;
    DestStore b = a.template move<Verbose, 8, &raw_alloc, &raw_free>
        (3, 1, &result);

    assert(result == opres::SUCCESS);
    assert(b.size() == 3);
    assert(Verbose::alive == 8); // 5 original + 3 moved

    assert(b[0].value == 10);
    assert(b[1].value == 20);
    assert(b[2].value == 30);

    // Validate source hasn't been destroyed yet, just moved-from
    assert(a[1].value == -1);
    assert(a[2].value == -1);
    assert(a[3].value == -1);

    // Ensure rest of source is intact
    assert(a[0].value == 0);
    assert(a[4].value == 40);

    std::cout << "--- Move partial segment successful ---\n";

    // Clean-up happens automatically via RAII
    return 0;
}

