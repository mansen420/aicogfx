#include "foo.h"

int main()
{
    //uhm what the fuck?
    //you are calling a function for initializing a primitive (why?),
    //which in fact blocks the execution of the whole program in I/O
    //such functions are, in virtually all cases, never a good idea to
    //have an API.
    //Especially for a task as simple as initializing a primitive type.
    double x{getDouble()};
    double y{getDouble()};
    char symbol{getChar()};
    
    //all the functions you have called here have side effects on the global
    //state of the program.
    //that's fine in the case of printing. But consider this, how often 
    //are you: actually printing in a real program? almost never.
    runEq(x, y, symbol);

    return 0;
}

