#ifndef _MEMOBJ_H_
#define _MEMOBJ_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "perf.h"

#define MEMOBJ_CACHE_NUM 512

struct memobj_pool {
    void *pool;
    int32_t memobj_num;
    int32_t last_pool_idx;
    size_t memobj_size;
    int32_t *cache;
    uint16_t cache_num;
    int16_t top;
    int32_t free_cnt;
    // ptr of func to get memobj
};

int
init_mpool(struct memobj_pool *mpool, void *memobjs, size_t memobj_size, const uint32_t memobj_num, const uint16_t cache_num)
{
    uint16_t i = 0;

    mpool->memobj_num = memobj_num;
    mpool->last_pool_idx = 0;
    mpool->memobj_size = memobj_size;
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
    mpool->free_cnt = memobj_num;
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

// TODO: Manage available memobjs with a list
static inline int32_t
memobj_get_stack(struct memobj_pool *mpool)
{
    if (unlikely(mpool->free_cnt <= 0)) {
        fprintf(stderr, "memobj_get_stack: no available memobjs\n");
        exit(EXIT_FAILURE);
    }
    int32_t cache_id = memobj_pop_stack(mpool);
    if (cache_id >= 0) {
        return cache_id;
    }

    int32_t mobj_idx = mpool->last_pool_idx++;
    if (mpool->last_pool_idx >= mpool->memobj_num) {
        mpool->last_pool_idx -= mpool->memobj_num; // mpool->last_pool_idx = 0;
    }

    mpool->free_cnt--;

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
        fprintf(stderr, "memobj_put_stack: failed to free a memobj without caching it\n");
        fprintf(stderr, "TODO: Manage available memobjs with a list");
        exit(EXIT_FAILURE);
    }

    mpool->free_cnt++;

    return;
}

// int
// memobj_get_bulk(struct memobj_pool *mpool);

#endif /* _MEMOBJ_H_ */
