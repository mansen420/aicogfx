#include <cstdlib>
#include "window.h"

int main()
{
    using namespace aicogfx;
    if(init() == opres::FAILURE)
        return EXIT_FAILURE;
    
    //hand control to window context 
    wndctx mainctx(600, 300, "Title");

    //context terminated, terminate engine 
    terminate();

    return EXIT_SUCCESS;
}
