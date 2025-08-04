#pragma once

#include "opres.h"
#include "debug.h"

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
    inline heap_t g_heaps[HEAPS_MAX];
    inline uint8_t g_heapsz=0;

    struct alignas(alignof(max_align_t)) 
        hdr_t //size:16, align:16 (64 bit)
    {
        uint32_t magic=0xC0FFEE;
        size_t bytes; 
        void* base;
        uint16_t flags=0;
        enum flagbits:uint8_t
        {
            DEDICATED=1<<0/*big alloc*/
        };
        uint8_t heapid;
    };

    float g_powerfactor=2.f;//consider making this a ratio of ints
    unsigned int g_linearbump=2*MB;
    unsigned int g_constantbump=32*MB;
    enum class alloc_strat: uint8_t
    {
        POWER, LINEAR, CONSTANT 
    };
    inline alloc_strat g_strat=alloc_strat::POWER;
    
    //assumes enough space after base
    inline void* align_up(void* base, size_t align)
    {
        return (void*)((uintptr_t)((char*)base+align-1) & ~(align-1));
    }
    //assumes enough space before base
    inline void* align_down(void* base, size_t align)
    {
        return (void*)((uintptr_t)((char*)base) & ~(align-1));
    }
    
    //assumes sufficient space before usrptr
    //at least sizeof(hdr_t)+alignof(hdr_t)-1
    inline hdr_t* header_addr(void* usrptr)
    {
        return (hdr_t*)align_down((char*)usrptr-sizeof(hdr_t),
            alignof(hdr_t));
    }
    hdr_t* gethdr(void* addr)
    {
        hdr_t* ptr= header_addr(addr);
        if(ptr->magic!=0xC0FFEE)
            return nullptr; //invalid addr
        return ptr;
    }
    inline bool is_pow2(size_t num)
    {
        return num /*not 0*/ && !(num & (num - 1)) /*power of 2*/;
    }
    //TODO most naive part of the alloctor
    void* heap_fit(heap_t& h, size_t maxbytes)
    {
        //TODO consider the case where we allocate X bytes at
        //Y alignment, then free the first allocation and make the
        //same request again. note how that second request might never
        //use the first allocation's space, due to the pessimistic nature
        //of maxbytes. consider "simulating" an allocation within a certain
        //wiggle room, (say, [userbytes, userbytes+worst_overhead])
            freenode_t* node=h.freelist;
        freenode_t* prev=nullptr;
        while(node)
        {
            if(hdr_t* header=gethdr(node); header)
                if(maxbytes<=header->bytes)
                {
                    //not free anymore, unlink
                    if(prev)
                        prev->next=node->next;
                    else
                        h.freelist=node->next;
                    return header->base;
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
    size_t grow(size_t base_size)
    {
        size_t bytes=0;
        switch(g_strat)
        {
        case alloc_strat::POWER: 
            bytes=(size_t)((double)g_powerfactor*(double)base_size);
            break;
        case alloc_strat::LINEAR:
            bytes=base_size+g_linearbump;
            break;
        case alloc_strat::CONSTANT:
            bytes=g_constantbump;
            break;
        }
        return bytes;
    }
    [[nodiscard]]heap_t* alloc_heap(size_t bytes) //page aligned
    {
        //HACK assume 4096, sane default on x86, also technically UB cast
        size_t page_aligned=(uintptr_t)align_up((void*)bytes, 4*KB);
        void* addr=malloc(page_aligned);
        if(addr)
        {
            g_heaps[g_heapsz++]={.bytes=page_aligned, .addr=addr, .offset=0, 
                .freelist=nullptr};
            return &g_heaps[g_heapsz-1];
        }
        else return nullptr;
    }
    void* malc(size_t bytes, size_t alignment=alignof(hdr_t), opres* res=nullptr)
    {
        if(!is_pow2(alignment)||(alignment<alignof(void*)))
        {
            if(res)
                *res=opres::ALIGN_ERR;
            return nullptr;
        }
        if(bytes<sizeof(freenode_t)) bytes=sizeof(freenode_t);
        //worst case, user pointer can end up in
        //base+(sizeof(hdr)+alignof(hdr-1))+(alignment-1), +1 makes sure this case 
        //lands at a valid address
        const size_t worst_overhead=sizeof(hdr_t)+(alignof(hdr_t)-1)+(alignment-1)+1;
        const size_t hdr_buffer=sizeof(hdr_t)+(alignof(hdr_t)-1);
        const bool largealloc=bytes>=64*MB;//heuristic
        const bool badalloc=g_strat==alloc_strat::CONSTANT&&
            bytes>g_constantbump;//dumbass
        if(largealloc||badalloc)//handle tyrant allocs 
        {
            void*addr=malloc(bytes+worst_overhead);
            if(!addr) 
            {
                if(res) *res=opres::MEM_ERR;
                return nullptr;
            }
            void* aligned=align_up((char*)addr+hdr_buffer, alignment);
            *(header_addr(aligned))={.magic=0xC0FFEE, 
                .bytes=bytes+((char*)aligned-(char*)addr), 
                .base=addr, .flags=hdr_t::DEDICATED, .heapid=0};
            if(res) *res=opres::SUCCESS;
            return aligned;
        }
        for(uint8_t i=0; i<g_heapsz; ++i)
        {
            heap_t& h=g_heaps[i];
            void* base=heap_fit(h, bytes+worst_overhead);
            if(!base)
                continue;
            void* usr_aligned=align_up((char*)base+hdr_buffer, alignment);
            *(header_addr(usr_aligned))={.magic=0xC0FFEE, 
                .bytes=bytes+((char*)usr_aligned-(char*)base),
                .base=base, .flags=0, .heapid=i};
            if(res) *res=opres::SUCCESS;
            return usr_aligned;
        }
        //no heap fits, allocate new heap
        if(g_heapsz>=HEAPS_MAX)
        {
            if(res) *res=opres::NO_HEAPS;
            return nullptr;
        }
        size_t next_size=g_heapsz==0?/*default init*/2*MB:
            grow(g_heaps[g_heapsz-1].bytes);
        while(bytes>=next_size/3)//heuristic, measure later
            next_size=grow(next_size);
        heap_t* h=alloc_heap(next_size);
        if(!h)
        {
            if(res) *res=opres::MEM_ERR;
            return nullptr;
        }
        void* base=heap_fit(*h, bytes+worst_overhead);
        void* usr_aligned=align_up((char*)base+hdr_buffer, alignment);
        *(header_addr(usr_aligned))={.magic=0xC0FFEE, 
            .bytes=bytes+((char*)usr_aligned-(char*)base), 
            .base=base, .flags=0, .heapid=(uint8_t)(g_heapsz-1)};
        if(res) *res=opres::SUCCESS;
        return usr_aligned;
    }
    //TODO does every allocation reserve enough to leave room for freenode_t on free?
    void rel(void* usraddr)noexcept
    {
        hdr_t* hdr=gethdr(usraddr);
        if(!hdr) return; //invalid address, can't do shit
        if(hdr->flags&hdr_t::DEDICATED)
        {
            free(hdr->base);
            return;
        }
        auto& heap=g_heaps[hdr->heapid];

        freenode_t* node=(freenode_t*)usraddr;
        node->next=heap.freelist;
        heap.freelist=node;
    }
}





