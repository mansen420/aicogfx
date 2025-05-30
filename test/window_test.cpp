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

        wndctx mainctx(600, 300, "Title");
    
        std::thread secondary_thread([&]()
            {
                while (!mainctx.looping())
                    std::this_thread::yield();
                sleep(2);
                mainctx.interrupt();
            });
        
        //hand control to window context 
        mainctx.loop();
        
        //catch interrupt
        sleep(2);
        
        //restart loop
        mainctx.loop();
        
        secondary_thread.join();
    }
    catch(opres code)
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
