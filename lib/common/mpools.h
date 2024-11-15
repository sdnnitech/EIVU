#ifndef _MPOOLS_H_
#define _MPOOLS_H_

#include <stdio.h>
#include <stdlib.h>

#include <memobj.h>

struct mpools {
    struct memobj_pool md_pool;
    struct memobj_pool pktbuf_pool;
    void *que_rx;
};

void
init_mpools(struct mpools *mpools, size_t mdbuf_size, size_t pktbuf_size, const uint32_t pktbuf_num, const uint32_t mdbuf_num, const uint32_t cache_num, void *memobjs, void *que_rx)
{
    void *memobjs_md, *memobjs_pktbuf;
    bool md_cache_is_stack, pktbuf_cache_is_stack;

    mpools->que_rx = que_rx;

#ifdef MD_MEMOBJ_CACHE_STACK
    md_cache_is_stack = true;
#else
    md_cache_is_stack = false;
#endif

#ifdef PKTBUF_MEMOBJ_CACHE_STACK
    pktbuf_cache_is_stack = true;
#else
    pktbuf_cache_is_stack = false;
#endif

    memobjs_pktbuf = memobjs;
    memobjs_md = (uint8_t *)memobjs + pktbuf_num * pktbuf_size;

    if (init_mpool(&mpools->md_pool, memobjs_md, mdbuf_size, mdbuf_num, cache_num, md_cache_is_stack) != 0) {
        fprintf(stderr, "init_mpool");
        exit(EXIT_FAILURE);
    }
    if (init_mpool(&mpools->pktbuf_pool, memobjs_pktbuf, pktbuf_size, pktbuf_num, cache_num, pktbuf_cache_is_stack) != 0) {
        fprintf(stderr, "init_mpool");
        exit(EXIT_FAILURE);
    }
}

void
fin_mpools(struct mpools *mpools, bool is_shm)
{
    if (is_shm) {
        fin_mpool(&mpools->md_pool, false);
        fin_mpool(&mpools->pktbuf_pool, false);
    } else {
        fin_mpool(&mpools->md_pool, true);
        fin_mpool(&mpools->pktbuf_pool, true);
    }
}

#endif
