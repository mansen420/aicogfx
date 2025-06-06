#include "engctx.h"
#include "wndctx.h"
#include "gfxctx.h"
#include "renderers.h"

int main()
{
    aico::engctx engine;
    
    aico::sys::wndctx mainctx(600, 300, "App", aico::sys::triangle);
    
    auto gfx = mainctx.makegfxctx({}, nullptr);
    
    mainctx.loop();

    return 0;
}
