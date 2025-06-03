#include "engctx.h"
#include "_engctx.h"
#include "sysinit.h"

uint32_t aico::engflags = 0;

aico::engctx::engctx() : flags(engflags)
{
    if(auto status = sys::init(); status != opres::SUCCESS)
        throw status;
}
aico::engctx::~engctx()noexcept
{
    sys::terminate();
}

