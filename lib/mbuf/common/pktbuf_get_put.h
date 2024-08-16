#ifndef _PKTBUF_GET_PUT_H_
#define _PKTBUF_GET_PUT_H_

#include <memobj.h>
#include <ring.h>
#include <stack.h>

#include <stdio.h>
#include <stdlib.h>

static inline int32_t
alloc_pktbuf(struct memobj_pool *mpool)
{
#ifdef PKTBUF_MEMOBJ_CACHE_STACK
    int32_t cache_id = pop((struct stack *)mpool->cache);
#else
    int32_t cache_id = dequeue((struct ring_buf *)mpool->cache);
#endif

    if (cache_id >= 0) {
        return cache_id;
    }

    int32_t mobj_idx = dequeue(mpool->ring);
    if (unlikely(mobj_idx < 0)) {
        fprintf(stderr, "alloc_pktbuf: no available memobjs\n");
        exit(EXIT_FAILURE);
    }

    return mobj_idx;
}

static inline int
free_pktbuf(struct memobj_pool *mpool, int32_t mobj_idx)
{
#ifdef PKTBUF_MEMOBJ_CACHE_STACK
    if (push((struct stack *)mpool->cache, mobj_idx) == -1) {
#else
    if (enqueue((struct ring_buf *)mpool->cache, mobj_idx) == -1) {
#endif
        // free a mobj without caching it
        if (unlikely(enqueue(mpool->ring, mobj_idx) < 0)) {
            fprintf(stderr, "free_pktbuf: enqueue: failed to free a memobj without caching it\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

#endif
