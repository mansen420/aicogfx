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

    std::cout << "--- Copy Test ---\n";

    {
        storage<Tracker> a(3, Tracker(99));
        assert(Tracker::alive == 3);

        auto b = a.copy();
        assert(Tracker::alive == 6);
        assert(b.size() == 3);
        assert(b[0].value == 99);

        auto c = a.copy<Tracker, 1, malloc, free>(2, 1);
        assert(c.size() == 2);
        assert(c[0].value == 99);

        std::cout << "Copy test passed.\n";
    }

    assert(Tracker::alive == 0);
    std::cout << "All Tracker instances destroyed cleanly.\n";

    return 0;
}
