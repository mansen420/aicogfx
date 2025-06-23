#include "wndctx.h"
#include "renderers.h"

int main()
{
    aico::starterparter engine;

    aico::sys::wndctx maincontext(500, 300, "app",
        aico::sys::flashing_red,
        aico::sys::wndctx::bits::DEBUGCTX);
    
    maincontext.loop();

    return 0;
}
