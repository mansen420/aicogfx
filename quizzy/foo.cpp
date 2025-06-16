#include <iostream>
#include "foo.h"

double getDouble()
{
    std::cout << "Enter a double value: ";
    double d{};
    std::cin >> d;

    return d;
}

char getChar()
{
    std::cout<< "Enter +, -, *, or /: ";
    char c{};
    std::cin >> c;

    return c;
}

void runEq(double one, double two, char letter)
{
    if(letter == '+')
    {
        std::cout << one << " " << letter<< " " << two << " is " << one + two << '\n';
    } 
    else if (letter == '-')
    {
        std::cout << one << " " << letter<< " " << two << " is " << one - two << '\n';
    }
    else if (letter == '*')
    {
        std::cout << one << " " << letter<< " " << two << " is " << one * two << '\n';
    }
    else if (letter == '/')
    {
        std::cout << one << " " << letter<< " " << two << " is " << one / two << '\n';
    }
}
