#pragma once

#include "opres.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace aico::sys
{
    constexpr size_t KB=1024;
    constexpr size_t MB=KB*KB;

    struct freenode_t
    {
        freenode_t* next;
    };
    struct heap_t
    {
        size_t bytes;
        void* addr;
        size_t offset=0;
        freenode_t* freelist=nullptr;
    };
    constexpr size_t HEAPS_MAX=UINT8_MAX;
    static heap_t g_heaps[HEAPS_MAX];
    static size_t g_heapsz=0;

    struct alignas(alignof(max_align_t)) hdr_t //size:15, align:16 (64 bit)
    {
        uint32_t magic=0xC0FFEE;
        size_t bytes;
        uint16_t flags;
        uint8_t heapid;
    };

    float g_powerfactor=2.f;
    unsigned int g_linearbump=2*MB;
    enum class alloc_strat: uint8_t
    {
        POWER, LINEAR, CONSTANT 
    };

    //asssumes [hdr][padding][ptr][addr]
    hdr_t* gethdr(void* addr)
    {
        hdr_t* ptr= *((hdr_t**)(addr)-1);
        if(ptr->magic!=0xC0FFEE)
            return nullptr; //invalid addr
        return ptr;
    }
    inline bool is_pow2(size_t num)
    {
        return num /*not 0*/ && !(num & (num - 1));
    }
    void* heap_fit(heap_t& h, size_t maxbytes)
    {
        freenode_t* node=h.freelist;
        freenode_t* prev=nullptr;
        while(node)
        {
            if(hdr_t* header=gethdr(node); header)
                if(maxbytes<=header->bytes)
                {
                    if(prev)
                        prev->next=node->next;
                    else
                        h.freelist=node->next;
                    return (void*)node;
                }
            prev=node;
            node=node->next;
        }
        size_t remain=h.bytes-h.offset;
        if(remain>=maxbytes)
        {
            void* addr=(char*)h.addr+h.offset;
            h.offset+=maxbytes;
            return addr;
        }
        return nullptr;
    }
    //assumes enough space after base
    inline void* align_up(void* base, size_t align)
    {
        return (void*)((uintptr_t)((char*)base+align-1) & ~(align-1));
    }
    void* malc(size_t bytes, size_t alignment, opres* res=nullptr)
    {
        if(!is_pow2(alignment)||(alignment%alignof(void*)!=0))
        {
            if(res)
                *res=opres::ALIGN_ERR;
            return nullptr;
        }
        /*
        [ alignof(hdr_t) padding ]          ← required to align the start (heap base)
        [ hdr_t ]                           ← metadata
        [ alignof(void*) padding ]          ← for storing backpointer safely
        [ void* backpointer ]
        [ alignment padding ]               ← align for the actual user data
        [ user data (bytes) ]
        */
        const size_t worst_overhead =
            alignof(hdr_t)-1 +          // pre-header padding
            sizeof(hdr_t) +              
            alignof(void*)-1 +          // pre-backptr padding
            sizeof(void*) +              
            (alignment-1);              // pre-user padding
        for(size_t i=0; i<g_heapsz; ++i)
        {
            heap_t& h=g_heaps[i];
            void* base=heap_fit(h, bytes+worst_overhead);
            if(!base)
                continue;
            
        }
    }
}
