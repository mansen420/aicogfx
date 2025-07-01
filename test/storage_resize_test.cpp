#include <iostream>
#include <string>
#include <cassert>
#include <type_traits>

#include "vector.h"

struct Hammer {
    std::string name;
    inline static int live_count = 0;

    Hammer(std::string n) : name(std::move(n)) {
        ++live_count;
        std::cout << "CTOR: " << name << "\n";
    }

    Hammer(const Hammer& other) : name(other.name + "_copy") {
        ++live_count;
        std::cout << "COPY: " << name << "\n";
    }

    Hammer(Hammer&& other) noexcept : name(std::move(other.name) + "_move") {
        ++live_count;
        std::cout << "MOVE: " << name << "\n";
    }

    ~Hammer() {
        --live_count;
        std::cout << "DTOR: " << name << "\n";
    }
};

int main() {
    using namespace aico; // if needed

    std::cout << "--- Creating empty ---\n";
    storage<Hammer, DYNAMIC> s(0); // Should allocate Mincpct

    assert(Hammer::live_count == 0);

    std::cout << "--- Resize to 4 ---\n";
    s.resize(4);
    for (size_t i = 0; i < 4; ++i)
        new (s.begin() + i) Hammer("h" + std::to_string(i));
    assert(Hammer::live_count == 4);

    std::cout << "--- Resize to 2 (shrink) ---\n";
    s.resize(2); // Should call 2 dtors
    assert(Hammer::live_count == 2);

    std::cout << "--- Resize to 6 (grow) ---\n";
    s.resize(6);
    for (size_t i = 2; i < 6; ++i)
        new (s.begin() + i) Hammer("h" + std::to_string(i));
    assert(Hammer::live_count == 6);

    std::cout << "--- Reserve for 20 ---\n";
    s.rsvcpct(20);
    assert(s._capacity >= 20);

    std::cout << "--- Resize to 0 (should destroy all) ---\n";
    s.resize(0);
    assert(Hammer::live_count == 0);

    std::cout << "--- Reserve 0 (should be no-op due to Mincpct) ---\n";
    s.rsvcpct(0);
    assert(s._capacity >= 8); // assuming Mincpct=8

    std::cout << "--- Resize and fill again ---\n";
    s.resize(3);
    for (size_t i = 0; i < 3; ++i)
        new (s.begin() + i) Hammer("re" + std::to_string(i));
    assert(Hammer::live_count == 3);

    std::cout << "--- Final cleanup ---\n";
    s.resize(0);
    assert(Hammer::live_count == 0);

    std::cout << "--- All tests passed ---\n";
    return 0;
}
