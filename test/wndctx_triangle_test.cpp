#include "wndctx.h"
#include "gfxctx.h"
#include "renderers.h"

int main()
{
    aico::starterparter engine;

    aico::sys::wndctx mainctx(600, 300, "App", aico::sys::triangle, aico::sys::wndctx::DEBUGCTX);
    
    auto gfx = mainctx.makegfxctx({}, nullptr);
    
    mainctx.loop();

    return 0;
}
