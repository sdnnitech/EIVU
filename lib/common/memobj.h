#ifndef _MEMOBJ_H_
#define _MEMOBJ_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <ring.h>
#include <stack.h>

#include "perf.h"

struct memobj_pool {
    void *pool;
    int32_t memobj_num;
    uint32_t memobj_size;
    void *cache;
    struct ring_buf *ring; // not local cache
};

int
init_mpool(struct memobj_pool *mpool, void *memobjs, uint32_t memobj_size, const uint32_t memobj_num, const uint32_t cache_num, bool is_stack)
{
    uint32_t i = 0;

    mpool->memobj_num = memobj_num;
    mpool->memobj_size = memobj_size;
    mpool->pool = memobjs;

    mpool->ring = init_ring(memobj_num, -1);
    if (mpool->ring == NULL)
        return -1;

    for (i = 0; i < memobj_num; i++) {
        if (enqueue(mpool->ring, i) < 0)
            return -1;
    }

    // init cache for memobj
    if (is_stack) {
        mpool->cache = init_stack(cache_num, -1);
    } else {
        mpool->cache = init_ring(cache_num, -1);
    }

    if (mpool->cache == NULL)
        return -1;

    return 0;
}

void
fin_mpool(struct memobj_pool *mpool, bool is_free_pool)
{
    if (is_free_pool)
        free(mpool->pool);
    free(mpool->ring);
    free(mpool->cache);
}

#endif /* _MEMOBJ_H_ */
