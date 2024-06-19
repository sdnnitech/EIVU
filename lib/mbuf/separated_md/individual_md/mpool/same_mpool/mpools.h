#ifndef _MPOOLS_H_
#define _MPOOLS_H_

#include <stdio.h>
#include <stdlib.h>

#include <memobj.h>

struct mpools {
    struct memobj_pool buf_pool;
};

void
init_mpools(struct mpools *mpools, size_t memobj_size, const uint32_t memobj_num, const uint32_t cache_num, bool is_shm, void *memobjs_shm)
{
    void *memobjs;
    bool memobj_cache_is_stack;

#if defined MD_MEMOBJ_CACHE_STACK || defined PKTBUF_MEMOBJ_CACHE_STACK
    memobj_cache_is_stack = true;
#else
    memobj_cache_is_stack = false;
#endif

    if (is_shm) {
        memobjs = memobjs_shm;
    } else {
        memobjs = calloc(memobj_num, memobj_size);
    }
    if (memobjs == NULL) {
        fprintf(stderr, "init_mpools");
        exit(EXIT_FAILURE);
    }

    if (init_mpool(&mpools->buf_pool, memobjs, memobj_size, memobj_num, cache_num, memobj_cache_is_stack) != 0) {
        fprintf(stderr, "init_mpool");
        exit(EXIT_FAILURE);
    }
}

void
fin_mpools(struct mpools *mpools, bool is_shm)
{
    if (is_shm)
        fin_mpool(&mpools->buf_pool, false);
    else
        fin_mpool(&mpools->buf_pool, true);
}

#endif
