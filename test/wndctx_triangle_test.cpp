#include "aico/engctx.h"
#include "aico/wndctx.h"
#include "aico/gfxctx.h"
#include "aico/renderers.h"

int main()
{
    aico::engctx engine;

    aico::sys::wndctx mainctx(600, 300, "App", aico::sys::triangle, aico::sys::wndctx::DEBUGCTX);
    
    auto gfx = mainctx.makegfxctx({}, nullptr);
    
    mainctx.loop();

    return 0;
}
