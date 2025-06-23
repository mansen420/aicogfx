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
        
        std::atomic<opres*> scdres{nullptr}; 
        std::thread scdthread([&scdres]()
                {
                    try
                    {
                        wndctx scdctx(300, 300, "BBB", flashing_red);
                        scdres.store(new opres{opres::SUCCESS});
                        scdctx.loop();
                    }
                    catch(opres code)
                    {
                        printf("Got code: %u\nTest FAILED!\n", code);
                        scdres.store(new opres{opres::FAILURE});
                    }
                });
        mainctx.loop();
        scdthread.join();

        opres res = *scdres.load();
        delete scdres.load();
        if(res != opres::SUCCESS)
            throw res;
    }
    catch(opres code)
    {
        printf("Got code: %u\nTest FAILED!\n", code);
        return EXIT_FAILURE;
    }
    printf("Test pass. Nice!\n");
    return EXIT_SUCCESS;
}
