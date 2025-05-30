/**
 * @file
 * @brief 
 */
#pragma once
namespace aicogfx::sys
{
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
