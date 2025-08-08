#include "aico/storage.h"
#include <iostream>
#include <cassert>

struct Dummy {
    int x;
    Dummy() : x(42) {}
    Dummy(int a) : x(a) {}
    Dummy(const Dummy& d) : x(d.x + 1) {}
};

using namespace::aico;
void test_default_ctor() {
    storage<Dummy, DYNAMIC> s(3);
    assert(s.size() == 3);
    for (size_t i = 0; i < s.size(); ++i)
        assert(s[i].x == 42);
    std::cout << "✅ default_ctor passed\n";
}

void test_uninitialized_ctor() {
    struct NoDefault {
        int y;
        NoDefault() = delete;
        explicit NoDefault(int x): y(x) {}
    };

    storage<NoDefault, DYNAMIC> s(5); // shouldn't default-construct anything
    std::cout << "✅ uninitialized_ctor passed (compile-time)\n";
}

void test_fill_ctor() {
    Dummy d(99);
    storage<Dummy, DYNAMIC> s(4, d);
    assert(s.size() == 4);
    for (size_t i = 0; i < s.size(); ++i)
        assert(s[i].x == d.x + 1); // because copy ctor bumps by 1
    std::cout << "✅ fill_ctor passed\n";
}

void test_custom_ctor() {
    storage<Dummy, DYNAMIC> s(3, 77);
    assert(s.size() == 3);
    for (size_t i = 0; i < s.size(); ++i)
        assert(s[i].x == 77);
    std::cout << "✅ custom_ctor passed\n";
}

void test_take_ownership_ctor() {
    Dummy* buffer = static_cast<Dummy*>(::operator new[](sizeof(Dummy)*3));
    for (int i = 0; i < 3; ++i)
        new (buffer + i) Dummy(i * 10);

    storage<Dummy, DYNAMIC> s(buffer, 3);
    assert(s.size() == 3);
    for (int i = 0; i < 3; ++i)
        assert(s[i].x == i * 10);
    std::cout << "✅ take_ownership_ctor passed\n";
}

void test_static_ctor() {
    storage<int, 3> s;
    static_assert(std::is_trivially_destructible_v<decltype(s)>);
    std::cout << "✅ static_ctor passed (compile-time)\n";
}

int main() {
    test_default_ctor();
    test_uninitialized_ctor();
    test_fill_ctor();
    test_custom_ctor();
    test_take_ownership_ctor();
    test_static_ctor();
    std::cout << "🎉 All constructor tests passed\n";
}
