/**
 * @file
 * @brief 
 */
#pragma once
#include <cstdint>
namespace aicogfx
{
    enum class opres : uint32_t
    {
        FAILURE, SUCCESS
    };
    
    
    extern uint32_t engine_flags;

    enum engine_flag_bits
    {
        INIT = 1 << 0
    };

    /**
     * @brief Initializes the engine.
     * This must be called before any other engine construct may be used.
     * Calling this after the engine has already been initialized will have no effect.
     * This may be used to re-initialize the engine after a call to aicogfx::terminate()
     * Engine initialization status is marked in aicogfx::engine_flags with the 
     * aicogfx::engine_flag_bits::INIT bit.
     *
     * @return aicogfx::opres::SUCCESS for success, 
     * consult return status codes otherwise.
     */
     opres init();
    /**
     * @brief Terminates engine resources. 
     * Calling this before the engine has been initialized has no effect.
     * No engine constructs may be used after calling this.
     */
    void terminate();

    /**
     * @class wndctx
     * @brief RAII wrapper responsible for initializing and terminating a 
     * system window context where rendering can happen. 
     * Only one context may be current per thread.
     *
     */
    struct wndctx
    {
        wndctx(int width, int height, char* title);
        wndctx(wndctx&&);
        wndctx(const wndctx&) = delete;
        wndctx& operator=(const wndctx&) = delete;
        wndctx& operator=(wndctx&&) = delete;
        ~wndctx();
    private:
        struct _impl;
        _impl* implptr = nullptr;
    };
}
