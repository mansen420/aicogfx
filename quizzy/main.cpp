#include "foo.h"

int main()
{
    double x{getDouble()};    
    double y{getDouble()};    
    char symbol{getChar()};
    
    runEq(x, y, symbol);

    return 0;
}

