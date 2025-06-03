/**
 * @file
 * @brief 
 */
#pragma once
#include <cstdint>

#include "opres.h"

namespace aico{struct gfxctx; struct gfxconf_t;}

namespace aico::sys
{
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
        struct renderer_t;
        /**
         * @struct frameinfo
         * @brief per-frame information, passed to the render function.
         */
        struct frameinfo {};
        /**
         * @brief information about the window context, may change depending
         * external events.
         */
        struct info{int width, height; const char* title; uint32_t flags;};
        
        typedef void(*render_callback)(const frameinfo&, gfxctx*, void*);

        wndctx(int width, int height, const char* title, renderer_t renderer,
        uint32_t flags = 0);
        wndctx(wndctx&&) noexcept;
        wndctx(const wndctx&) = delete;
        wndctx& operator=(const wndctx&) = delete;
        wndctx& operator=(wndctx&&) = delete;
        ~wndctx() noexcept;
        
        render_callback renderfnc = nullptr;
        void* stateptr = nullptr;
        
        gfxctx* makegfxctx(gfxconf_t, opres* res)const noexcept;

        enum bits
        {
            DEBUGCTX = 1 << 0,
        };
        
        /**
         * @brief implement later, should return boolean-like values of the
         * context
         *
         * @return 
         */
        uint32_t getflags()const noexcept;

        /**
         * @brief 
         *
         * @return 
         */
        info getinfo()const noexcept;

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
        info _info;
        frameinfo _framedata;
        struct _impl;
        _impl* implptr = nullptr;
    };
    struct wndctx::renderer_t
    {
        const wndctx::render_callback fnc;
        void* const stateptr;
    };
}
