#pragma once

#include "opres.h"
#include "wndctx.h"

#include <cstdint>
#include <iostream>
#include <ostream>
#include <vector>
#include <optional>

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
        [[nodiscard]]buf_t bufalloc(bufinfo, const void*)const noexcept;
        void free(buf_t&)const noexcept;
        opres bufdata(const buf_t&, const void* data, size_t size, size_t buf_offset)
            const noexcept;
        
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
            /**
             * @brief An arrray of buffers to be bound to vertex attributes.
             */
            std::vector<bindinfo> buffers;
            /**
             * @brief An array of attribute descriptions.
             */
            std::vector<attribinfo> attribs;
            /**
             * @brief Type of index buffer indices. U8 is an unsigned 8 bit integer, etc.
             */
            enum class indexfmt : uint8_t
            {
                U8, U16, U32
            };
            /**
             * @brief An optional index buffer. 
             *
             * @detail The index buffer is used to order vertex attribute information.
             * i.e., rather than vertex attribute X getting the data at buffer index X, vertex attribute X may index
             * the buffer data arbitrarily.
             * The index buffer is such that index X tells which index vertex X will get its data from.
             * 
             * The indexfmt object tells the type of the index buffer indices.
             */
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
        /**
         * @brief 
         *
         * @note expensive operation.
         *
         * @return 
         */
        opres bind(const vtxlayout_t&)const noexcept;
        void free(vtxlayout_t&)const noexcept;
        
        /*PROGRAM*/
        struct stageinfo
        {
            /**
             * @brief Null terminated C string
             */
            const char* source;
            enum class type: uint8_t
            {
                VERT, FRAG, TESC, TESE, GEOM, COMP
            };
            /**
             * @brief Type of shader stage. Valid types are in stageinfo::type.
             */
            type T;
        };
        struct stage_t
        {
            stageinfo::type type()const noexcept;
        private:
            stage_t(stageinfo);
            stageinfo::type _type;

            struct handle_t;
            handle_t* _hnd;
        };
        [[nodiscard]]stage_t compile(const stageinfo&, opres*)const noexcept;
        void free(stage_t&)const noexcept;
        struct program_t
        {
        private:
            program_t();
            struct handle_t;
            handle_t* _hnd;
        };
        [[nodiscard]]program_t link(const std::vector<stage_t>&, opres*)const noexcept;
        opres bind(program_t)const noexcept;
        void free(program_t&)const noexcept;
    private:
        gfxctx(gfxconf_t);

        opres _init(const sys::wndctx::info&) noexcept;
        
        struct _impl;
        _impl* implptr;
    public:
        [[nodiscard]]_impl* getimpl()noexcept;
    };
}
