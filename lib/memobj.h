#ifndef _MEMOBJ_H_
#define _MEMOBJ_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "perf.h"

#define MBUF_HEADROOM_SIZE (CACHE_LINE_SIZE * 2)

#define MEMOBJ_SIZE (2048 + MBUF_HEADROOM_SIZE)
#define MEMOBJ_NUM 163456

#define MEMOBJ_CACHE_NUM 512

typedef uint8_t memobj[MEMOBJ_SIZE];

struct memobj_pool {
    int32_t memobj_num;
    int32_t last_pool_idx;
    memobj *pool;
    uint16_t pad;
    uint16_t cache_num;
    int16_t top;
    int32_t *cache;
    // ptr of func to get memobj
};

int
init_mpool(struct memobj_pool *mpool, memobj *memobjs, const uint32_t memobj_num, const uint16_t cache_num)
{
    uint16_t i = 0;

    mpool->memobj_num = memobj_num;
    mpool->last_pool_idx = 0;
    mpool->pool = memobjs;

    // init cache for memobj
    mpool->cache = calloc(cache_num, sizeof(int32_t));
    if (mpool->cache == NULL) {
        return -1;
    }
    for (i = 0; i < cache_num; i++) {
        mpool->cache[i] = -1;
    }
    mpool->cache_num = cache_num;
    mpool->top = -1;
    return 0;
}

void
fin_mpool(struct memobj_pool *mpool)
{
    free(mpool->cache);
}

static inline int32_t
memobj_pop_stack(struct memobj_pool *mpool)
{
    if (mpool->top == -1 || mpool->cache_num == 0) {
        return -1;
    }

    int32_t mobj_idx = mpool->cache[mpool->top--];
    return mobj_idx;
}

static inline int32_t
memobj_get_stack(struct memobj_pool *mpool)
{
    int32_t cache_id = memobj_pop_stack(mpool);
    if (cache_id >= 0) {
        return cache_id;
    }

    int32_t mobj_idx = mpool->last_pool_idx++;
    if (mpool->last_pool_idx >= mpool->memobj_num) {
        mpool->last_pool_idx -= mpool->memobj_num; // mpool->last_pool_idx = 0;
    }
    return mobj_idx;
}

static inline int
memobj_push_stack(struct memobj_pool *mpool, int32_t mobj_idx)
{
    if (mpool->cache_num == 0 || mpool->top == mpool->cache_num - 1) {
        return -1;
    }

    mpool->cache[++mpool->top] = mobj_idx;
    return 0;
}

static inline void
memobj_put_stack(struct memobj_pool *mpool, int32_t mobj_idx)
{
    if (memobj_push_stack(mpool, mobj_idx) == -1) {
        // free a mobj without caching it
    }

    return;
}

// int
// memobj_get_bulk(struct memobj_pool *mpool);

#endif /* _MEMOBJ_H_ */
