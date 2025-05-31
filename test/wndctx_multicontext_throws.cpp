#include <cstdio>
#include <cstdlib>
#include <thread>
#include <unistd.h>
#include "wndctx.h"
#include "engctx.h"
#include "opres.h"

int main()
{
    using namespace aicogfx;
    using namespace aicogfx::sys;
    try
    {
        engctx engine_guard;//enforces correct destruction order 

        wndctx mainctx(600, 300, "AAA");
        
        wndctx secondctx(300, 300, "BBB");

        mainctx.loop();
    }
    catch(opres code)
    {
        printf("Got code: %u\nTest seems successful.\n", code);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
