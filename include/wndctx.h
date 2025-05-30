/**
 * @file
 * @brief 
 */
#pragma once
#include <cstdint>
namespace aicogfx::sys
{
    enum class opres : uint32_t
    {
        SUCCESS, FAILURE, CONTEXT_CURRENT
    };
    
    
    extern uint32_t engine_flags;

    enum engine_flag_bits
    {
        INIT = 1 << 0
    };
    
    struct engctx
    {
        engctx();
        ~engctx()noexcept;
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
    void terminate()noexcept;

    /**
     * @class wndctx
     * @brief RAII wrapper responsible for initializing and terminating a 
     * system window context where rendering can happen.
     *
     * Note that only one context may be current per thread,
     * i.e., you can only own one wndctx object per thread.
     * If a context is already current, 
     * then constructing this object will throw.
     */
    struct wndctx
    {
        wndctx(int width, int height, const char* title);
        wndctx(wndctx&&) noexcept;
        wndctx(const wndctx&) = delete;
        wndctx& operator=(const wndctx&) = delete;
        wndctx& operator=(wndctx&&) = delete;
        ~wndctx() noexcept;
        
        /**
         * @brief Returns whether the context is in-loop
         */
        bool looping()const noexcept;
        /**
         * @brief Starts execution of context loop. 
         * Beware that this takes control of the program flow from the 
         * calling thread, until either the window is closed,
         * or wndctx::interrupt is called from an external thread.
         */
        void loop();
        /**
         * @brief Progrmmatic way to interrupt window context loop.
         * Thread safe.
         * Once interrupted, the loop will hand control back to the
         * calling thread, and may be called again.
         */
        void interrupt() noexcept;
    private:
        struct _impl;
        _impl* implptr = nullptr;
    };
}
