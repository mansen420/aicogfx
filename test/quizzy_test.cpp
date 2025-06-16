#include "quizzy.h"
#include <cstdlib>

int main()
{
    if(aico::evaleq(1.0, 2.0, aico::operation_t::SUB) != -1.0)
        return EXIT_FAILURE;
    aico::puteq(1.0, 2.0, aico::operation_t::SUB);
    return EXIT_SUCCESS;
}
