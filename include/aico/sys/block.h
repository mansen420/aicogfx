#pragma once

#include <cstddef>
#include <cstring>
#include <new>

namespace aico::sys
{
    typedef void*(*alloc_fn_t)(size_t, size_t, void*);
    typedef void(*dealloc_fn_t)(void*, void*);

    template<alloc_fn_t Alloc, dealloc_fn_t Dealloc>
    class block
    {
    private:
        void*  _addr;
        size_t _algn;
        size_t _cpct;
        void* _stptr;

        void _init(size_t bytes, size_t align, void* state=nullptr)
        {
            _addr=Alloc(bytes, align, state);
            if(!_addr)throw std::bad_alloc(); //replace this with our own exception types later
            _stptr=state;
            _cpct=bytes;
            _algn=align;
        }
        void _nullify()noexcept
        {
            this->_addr=nullptr;
            this->_cpct=0;
            this->_algn=0;
            this->_stptr=nullptr;
        }
        void _steal(block& other)noexcept
        {
            this->_addr=other._addr;
            this->_algn=other._algn;
            this->_cpct=other._cpct;
            this->_stptr=other._stptr;
            
            other._nullify();
        }
    public:
        block()=delete;
        block(size_t bytes, void* state=nullptr, size_t align=alignof(std::max_align_t)) 
        {
            if(bytes==0) this->_nullify();
            else this->_init(bytes, align, state);
        }
        block(const block&)=delete;
        block& operator=(const block&)=delete;
        block(block&& other)noexcept
        {
            this->_steal(other);
        }
        block& operator=(block&& other)noexcept
        {
            this->free();
            
            this->_steal(other);
            
            return *this;
        }
        void free()noexcept
        {
            if(!_addr) return; //protect from double free
            Dealloc(_addr, _stptr);

            this->_nullify();
        }
        void* addr()noexcept{return _addr;}
        const void* addr()const noexcept{return _addr;}
        size_t bytes()const noexcept{return _cpct;}
        size_t alignment()const noexcept{return _algn;}
        ~block()noexcept
        {
            this->free();
        }
    };

    template<size_t N, size_t Algn=alignof(std::max_align_t)>
    class stack_block
    {
    private:
        alignas(Algn) char _buf[N];
    public:
        const void* addr()const noexcept{return _buf;};
        void* addr()noexcept{return _buf;}
        constexpr size_t bytes()noexcept{return N;}
        constexpr size_t alignment()noexcept{return Algn;}

        stack_block(stack_block&&)=delete;
        stack_block(const stack_block&)=delete;
        stack_block& operator=(stack_block&&)=delete;
        stack_block& operator=(const stack_block&)=delete;
    };
}
