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
        
        std::atomic<opres*> scdres{nullptr}; 
        std::thread scdthread([&scdres]()
                {
                    try
                    {
                        wndctx scdctx(300, 300, "BBB");
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
