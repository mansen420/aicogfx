#include <iostream>
#include <stdexcept>
#include <cassert>

#include "storage.h"

using namespace::aico;

// Type that throws on copy
struct ThrowCopy {
    ThrowCopy() = default;
    ThrowCopy(const ThrowCopy&) {
        std::cout << "ThrowCopy copy constructor called\n";
        throw std::runtime_error("Copy failed");
    }
    ThrowCopy& operator=(const ThrowCopy&) = default;
};

// Type that throws on move
struct ThrowMove {
    ThrowMove() = default;
    ThrowMove(ThrowMove&&) {
        std::cout << "ThrowMove move constructor called\n";
        throw std::runtime_error("Move failed");
    }
    ThrowMove& operator=(ThrowMove&&) = default;
};

// Type with no default constructor
struct NoDefault {
    NoDefault(int x) : x(x) {}
    int x;
};

// Type with deleted copy constructor
struct NoCopy {
    NoCopy() = default;
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
};

// Type with deleted move constructor
struct NoMove {
    NoMove() = default;
    NoMove(NoMove&&) = delete;
    NoMove& operator=(NoMove&&) = delete;
};

// Type that logs everything
struct Verbose {
    Verbose() { std::cout << "Verbose()\n"; }
    Verbose(const Verbose&) { std::cout << "Verbose(const&)\n"; }
    Verbose(Verbose&&) { std::cout << "Verbose(&&)\n"; }
    Verbose(int x) { std::cout << "Verbose(int)\n"; (void)x; }
    ~Verbose() { std::cout << "~Verbose()\n"; }
};

int main() {
    try {
        std::cout << "\n--- ThrowCopy Test ---\n";
        storage<ThrowCopy> a(3, ThrowCopy());
    } catch (...) {
        std::cout << "Caught ThrowCopy exception as expected.\n";
    }

    try {
        std::cout << "\n--- ThrowMove Test ---\n";
        storage<ThrowMove> b(3, ThrowMove());
    } catch (...) {
        std::cout << "Caught ThrowMove exception as expected.\n";
    }

    try {
        std::cout << "\n--- NoDefault Test ---\n";
        storage<NoDefault> c(3, 42); // should work
    } catch (...) {
        std::cout << "Failed NoDefault construction.\n";
    }

    try {
        std::cout << "\n--- NoCopy Test ---\n";
        storage<NoCopy> d(3); // should work for default construction
    } catch (...) {
        std::cout << "Failed NoCopy construction.\n";
    }

    try {
        std::cout << "\n--- NoMove Test ---\n";
        storage<NoMove> e(3); // same as above
    } catch (...) {
        std::cout << "Failed NoMove construction.\n";
    }

    try {
        std::cout << "\n--- Verbose Test ---\n";
        storage<Verbose> f(3, 99);
    } catch (...) {
        std::cout << "Failed Verbose construction.\n";
    }

    std::cout << "\nAll tests finished.\n";
    return 0;
}
