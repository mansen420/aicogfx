/**
 * @file
 * @brief 
 */
#pragma once
namespace aicogfx::sys
{
    struct renderer_t;
    /**
     * @class wndctx
     * @brief RAII wrapper responsible for initializing and terminating a 
     * system window context where rendering can happen.
     *
     * Note that only one context may be current per thread,
     * i.e., you can only own one valid wndctx object per thread.
     * If a context is already current, 
     * then constructing this object will throw.
     */
    struct wndctx
    {
        struct frameinfo {};
        typedef void(*render_callback)(const frameinfo&, void*);

        wndctx(int width, int height, const char* title, renderer_t renderer);
        wndctx(wndctx&&) noexcept;
        wndctx(const wndctx&) = delete;
        wndctx& operator=(const wndctx&) = delete;
        wndctx& operator=(wndctx&&) = delete;
        ~wndctx() noexcept;
        
        render_callback renderfnc = nullptr;
        void* stateptr = nullptr;

        /**
         * @brief Returns whether the context is in-loop. Thread safe.
         */
        bool looping()const noexcept;
        /**
         * @brief Starts execution of context loop. 
         * Beware that this takes control of the program flow from the 
         * calling thread, until either the window is closed,
         * or wndctx::interrupt is called from an external thread.
         * 
         * This function will internally call wndctx::renderfnc, which
         * *must* be a valid pointer.
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
        frameinfo _framedata;
        struct _impl;
        _impl* implptr = nullptr;
    };
    struct renderer_t
    {
        const wndctx::render_callback fnc;
        void* const stateptr;
    };
}
