#include "engctx.h"
#include "opres.h"
#include "wndctx.h"
#include "gfxctx.h"
#include "renderers.h"
#include <cstdlib>
#include <iostream>

int main()
{
    aico::engctx engine;

    aico::sys::wndctx mainctx(600, 300, "App", aico::sys::flashing_red, aico::sys::wndctx::DEBUGCTX);
    
    aico::opres res;
    aico::gfxctx* graphics = mainctx.makegfxctx({std::cerr, std::cout}, &res);
    if(res != aico::opres::SUCCESS)
        return EXIT_FAILURE;

    //do smth useful with graphics

    mainctx.loop();

    return 0;
}
