#include "storage.h"
#include <iostream>
#include <type_traits>
#include <cassert>

using namespace aico;

struct Tracker {
    static int alive;
    int value;

    Tracker(int v = 0) : value(v) { ++alive; }
    Tracker(const Tracker&other): value(other.value) { ++alive; } // not used
    Tracker(Tracker&&other)noexcept: value(other.value) { ++alive; }

    ~Tracker() { --alive; }
};

int Tracker::alive = 0;

int main() {
    std::cout << "--- Move Constructor Test ---\n";

    {
        storage<Tracker> a(3, Tracker(42)); // init with values
        assert(Tracker::alive == 3);

        storage<Tracker> b(std::move(a)); // move construct
        assert(Tracker::alive == 3); // still same objects, ownership transferred

        assert(a.size() == 0 || a.begin() == nullptr); // moved-from should be inert
        assert(b.size() == 3);
        assert(b[0].value == 42);

        std::cout << "Move construction successful.\n";
    }

    assert(Tracker::alive == 0); // all destroyed

    std::cout << "--- Deleted Copy Test ---\n";
    static_assert(!std::is_copy_constructible_v<storage<Tracker>>, "Copy constructor should be deleted");
    static_assert(!std::is_copy_assignable_v<storage<Tracker>>, "Copy assignment should be deleted");

    std::cout << "All auxiliary constructor tests passed.\n";
}
