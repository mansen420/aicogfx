#include <cstdlib>
#include <thread>
#include <unistd.h>
#include "window.h"

int main()
{
    using namespace aicogfx::sys;

    if(init() == opres::FAILURE)
        return EXIT_FAILURE;
    try
    {
        engctx engine_guard;//enforces correct destruction order 

        wndctx mainctx(600, 300, "Title");
    
        std::thread secondary_thread([&]()
            {
                while (mainctx.looping())
                {
                    mainctx.interrupt();
                }
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
