#include <cstdio>
#include <iostream>

struct myInt 
{
    int64_t value;
    myInt(int64_t value) : value(value) {}
    operator int64_t() { std::cout << "Gimme int: "; int x; std::cin >> x; return x; }
};

int main()
{
    int64_t x = myInt(400);

    printf("%lu\n", x);
    return 0;
}
