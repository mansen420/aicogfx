#include <cstdio>
#include <cstdlib>
#include <thread>
#include <unistd.h>
#include "wndctx.h"
#include "opres.h"
#include "renderers.h"

int main()
{
    using namespace aico;
    using namespace aico::sys;
    try
    {
        starterparter engine_guard;//enforces correct destruction order 

        wndctx mainctx(600, 300, "AAA", flashing_red);
        
        wndctx secondctx(300, 300, "BBB", flashing_red);

        mainctx.loop();
    }
    catch(opres code)
    {
        printf("Got code: %u\nTest seems successful.\n", code);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
