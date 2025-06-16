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
    //1. [SAFETY] what if letter is none of these?
    //2. [PERF, CLARITY] using char to signal the op is cute, 
    //but unsafe. consider an enum.
    //C++ has enum class, which is type-safe (normal enums are just ints).
    //see: reference implementation.
    //
    //3. [API HYGEINE] your function is using standard I/O in a way that 
    //is invisible to the caller
    //this is unperformant or dangerous under multi-threading scnearios.
    //you don't _have_ to direct the result to stdout. what if the user has his 
    //own stream? what if this is running inside an embedded device _without_ 
    //a stdout stream? what if you don't even have the standard library?
    //
    //my suggestion: make two interfacs, one where you return the result via the
    //result via the function, and one where the user may optionally pass in a 
       //std::ostream reference/pointer (you can't pass these bad boys by value).
    //
    //you have two options here.
    //either make two functions
    //1. double eval(double x, double y, operation_t op);
    //2. void puteq(double x, double y, operation_t op, std::ostream& stream);
    //or combine them (C style):
    //double eval(double x, doubley, operation_t op, std::ostream* stream = nullptr);
    //last param is optional, in the code, check if == nullptr, print accordingly.

    //a reference implementation is provided. check /src /include and /test
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
