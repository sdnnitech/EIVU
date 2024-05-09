#ifndef _MEMOBJ_H_
#define _MEMOBJ_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "perf.h"

struct ring_buf {
    int32_t *ptr;
    int32_t max;
    int32_t num;
    int32_t front;
    int32_t rear;
};

struct memobj_pool {
    void *pool;
    int32_t memobj_num;
    size_t memobj_size;
    int32_t *cache;
    int32_t cache_num;
    int32_t top;
    struct ring_buf ring;
    // ptr of func to get memobj
};

static inline int
enqueue(struct ring_buf *ring, int32_t data)
{
    if (ring->num == ring->max)
        return -1;

    ring->ptr[ring->rear++] = data;
    ring->num++;

    return 0;
}

static inline int32_t
dequeue(struct ring_buf *ring)
{
    if (ring->num <= 0)
        return -1;
    
    int32_t data = ring->ptr[ring->front++];
    ring->num--;

    return data;
}

int
init_mpool(struct memobj_pool *mpool, void *memobjs, size_t memobj_size, const uint32_t memobj_num, const uint32_t cache_num)
{
    uint32_t i = 0;

    mpool->memobj_num = memobj_num;
    mpool->memobj_size = memobj_size;
    mpool->pool = memobjs;

    mpool->ring.ptr = calloc(memobj_num, sizeof(int32_t));
    if (mpool->ring.ptr == NULL) {
        return -1;
    }
    mpool->ring.max = memobj_num;
    mpool->ring.num = 0;
    mpool->ring.front = 0;
    mpool->ring.rear = 0; 
    for (i = 0; i < memobj_num; i++) {
        if (enqueue(&mpool->ring, i) < 0)
            return -1;
    }

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
    free(mpool->ring.ptr);
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

    int32_t mobj_idx = dequeue(&mpool->ring);
    if (unlikely(mobj_idx < 0)) {
        fprintf(stderr, "memobj_get_stack: no available memobjs\n");
        exit(EXIT_FAILURE);
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
        if (enqueue(&mpool->ring, mobj_idx) < 0) {
            fprintf(stderr, "memobj_put_stack: enqueue: failed to free a memobj without caching it\n");
            exit(EXIT_FAILURE);
        }
    }
}

// int
// memobj_get_bulk(struct memobj_pool *mpool);

#endif /* _MEMOBJ_H_ */
