#include "wndctx.h"
#include "renderers.h"
#include "engctx.h"

int main()
{
    aico::engctx engine;

    aico::sys::wndctx maincontext(500, 300, "app",
        aico::sys::flashing_red,
        aico::sys::wndctx::bits::DEBUGCTX);
    
    maincontext.loop();

    return 0;
}
