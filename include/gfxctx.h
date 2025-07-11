#pragma once

#include "opres.h"
#include "wndctx.h"

#include <cstdint>
#include <iostream>
#include <ostream>
#include <vector>
#include <optional>
#include <string>

namespace aico
{
    struct gfxconf_t
    {
        std::ostream& errlog = std::cerr;
        std::ostream& inflog = std::cout;
    };

    struct gfxctx
    {
    public:
        friend struct sys::wndctx;

        gfxctx(const gfxctx&) = delete;
        gfxctx(gfxctx&&) noexcept;
        ~gfxctx() noexcept;
        
        /*BUFFER*/
        struct bufinfo
        {
            size_t size;
            size_t stride;
        };
        struct buf_t
        {
            bufinfo info();
            ~buf_t()=default;
        private:
            friend struct gfxctx;

            bufinfo _info;
            buf_t(const bufinfo);

            struct handle_t;
            handle_t* _hnd;
        };
        [[nodiscard]]buf_t bufalloc(bufinfo, const void* =nullptr, opres* =nullptr)
            const noexcept;
        opres bufdata(const buf_t&, const void* data, size_t size, size_t buf_offset)
            const noexcept;
        void free(buf_t&)const noexcept;

        /*VTX LAYOUT*/
        struct bindinfo
        {
            buf_t buffer;
            unsigned int idx, offset = 0;
        };
        struct attribinfo
        {
            unsigned int idx, size, reloffst, bindidx;
            enum class type : uint8_t
            {
                FLOAT, HALF_FLT, DOUBLE_FLT
            };
            type T;
        };
        struct vtxlayout_info
        {
            std::vector<bindinfo> buffers;
            std::vector<attribinfo> attribs;
            enum class indexfmt : uint8_t
            {
                U8, U16, U32
            };
            std::optional<std::pair<buf_t, indexfmt>> indexbuf_fmt = std::nullopt;
        };
        struct vtxlayout_t
        {
            vtxlayout_info info();
            ~vtxlayout_t()=default;
        private:
            friend struct gfxctx;

            vtxlayout_info _info;
            vtxlayout_t(vtxlayout_info);

            struct handle_t;
            handle_t* _hnd;
        };
        [[nodiscard]]vtxlayout_t make_vtxlayout(vtxlayout_info)const noexcept;
        opres bind(const vtxlayout_t&)const noexcept;
        void free(vtxlayout_t&)const noexcept;
        
        /*PROGRAM*/
        struct stageinfo
        {
            const char* src; //GLSL
            enum class type: uint8_t
            {
                VERT, FRAG, TESC, TESE, GEOM, COMP
            };
            type T;
            int length = -1; //-1 indicates null-terminated string
        };
        struct shader_t
        {
            stageinfo::type type()const noexcept;
        private:
            friend struct gfxctx;
            shader_t(stageinfo);

            stageinfo::type _type;

            struct handle_t;
            handle_t* _hnd;
        };
        struct program_t
        {
        private:
            friend struct gfxctx;
            program_t();
            struct handle_t;
            handle_t* _hnd;
        };
        [[nodiscard]]shader_t compile(stageinfo, opres* =nullptr)const noexcept;
        [[nodiscard]]program_t link(const std::vector<shader_t>&, opres* =nullptr)const 
            noexcept;
        //TODO this is an illusion, there is no global state, make user
        //pass in own renderpass state explicitly, vulkan-style
        opres bind(program_t)const noexcept;
        void free(program_t&)const noexcept;
        void free(shader_t&)const noexcept;
    private:
        gfxctx(gfxconf_t);

        opres _init(const sys::wndctx::info&) noexcept;
        
        struct _impl;
        _impl* implptr;
    public:
        [[nodiscard]]_impl* getimpl()noexcept;
    };
}
