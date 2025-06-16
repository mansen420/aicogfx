#include "quizzy.h"
#include <cmath>

[[nodiscard]] double aico::evaleq(double x, double y, operation_t op)noexcept
{
    switch (op) 
    {
        case operation_t::ADD:
            return x + y;
        case operation_t::SUB:
            return x - y;
        case operation_t::MUL:
            return x * y;
        case operation_t::DIV:
            if(y == 0.0)
                return NAN; //undefined
            return x / y;
        default: //if op is none of the above. Impossible with enum class.
            return NAN; //this path should not be reachable, so it's here just to give you an idea.
    }
}

void aico::puteq(double x, double y, operation_t op, std::ostream& stream)
{
    double res = evaleq(x, y, op);
    char operation{};
    switch (op) 
    {
        case operation_t::ADD:
            operation = '+';
            break;
        case operation_t::SUB:
            operation = '-';
            break;
        case operation_t::MUL:
            operation = '*';
            break;
        case operation_t::DIV:
            operation = '/';
            break;
    }
    stream << x << " " << operation << " " << y << " = " << res << "\n";
}
