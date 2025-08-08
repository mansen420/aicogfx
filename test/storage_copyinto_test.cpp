#include <cassert>
#include <iostream>
#include "aico/storage.h"

struct Tracker {
    static int alive;
    int value;

    Tracker(int v = 0) : value(v) { ++alive; }
    Tracker(const Tracker& o) : value(o.value) { ++alive; }
    ~Tracker() { --alive; }

    friend std::ostream& operator<<(std::ostream& os, const Tracker& t) {
        return os << "Tracker(" << t.value << ")";
    }
};

int Tracker::alive = 0;

int main() {
    using namespace aico;

    std::cout << "--- Copy Into Test ---\n";

    {
        storage<Tracker> src(3, Tracker(10));
        storage<Tracker> dst(5, Tracker(99));

        assert(Tracker::alive == 8);

        // Overwrite elements 1-3 in dst with values from src[0-2]
        auto result = src.copy(3, dst, 1, 0, false);
        assert(result == opres::SUCCESS);
        assert(dst[1].value == 10);
        assert(dst[2].value == 10);
        assert(dst[3].value == 10);

        std::cout << "In-place copy without initialization passed.\n";
    }

    assert(Tracker::alive == 0);
    std::cout << "All objects destroyed cleanly.\n";

    // Test with raw uninitialized memory
    {
        Tracker* buf = (Tracker*)malloc(sizeof(Tracker) * 3);
        std::uninitialized_fill_n(buf, 3, Tracker(7));
        storage<Tracker, DYNAMIC, false, 8, &malloc, &free> 
            src(buf, 3);

        Tracker* rawdst = (Tracker*)malloc(sizeof(Tracker) * 3);
        storage<Tracker, DYNAMIC, false, 8, &malloc, &free> 
            dst(rawdst, 3);
        opres res = src.copy(3, dst, 0,
                0, true);

        assert(res == opres::SUCCESS);
        assert(dst[0].value == 7);
        assert(dst[1].value == 7);
        assert(dst[2].value == 7);

        std::cout << "In-place copy *with* initialization passed.\n";
    }

    assert(Tracker::alive == 0);
    std::cout << "All objects destroyed cleanly (again).\n";

    return 0;
}
