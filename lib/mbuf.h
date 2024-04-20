#ifndef _MBUF_H_
#define _MBUF_H_

#include <stdint.h>

#include "memobj.h"

// #define BUF_NUM 163456
// #define BUF_SIZE 2048

struct metadata {
    uint32_t pkt_len;
    uint16_t port;
    uint16_t pad;
};

struct mbuf_ptr {
    int32_t mbuf_idx;
    int32_t pad;
    struct memobj_pool *mpool;
#ifdef MDQUE
    struct desc *md;
#else
    struct metadata *md;
#endif
    uint8_t *pkt;
};

static inline void
reset_mbptr(struct mbuf_ptr *mbptr, uint32_t buf_idx, struct memobj_pool *mpool)
{
    mbptr->mbuf_idx = buf_idx;
    mbptr->md = (struct metadata *)((uint8_t *)&mpool->pool[mbptr->mbuf_idx] + CACHE_LINE_SIZE - sizeof(struct metadata));
    mbptr->pkt = (uint8_t *)(mbptr->md + 1);
    mbptr->mpool = mpool;
}

static inline void
mbuf_alloc(struct mbuf_ptr *mbptr, struct memobj_pool *mpool)
{
    mbptr->mbuf_idx = memobj_get_stack(mpool);
    reset_mbptr(mbptr, mbptr->mbuf_idx, mpool);
}

static inline void
mbuf_free(struct memobj_pool *mpool, struct mbuf_ptr *mbptr)
{
    memobj_put_stack(mpool, mbptr->mbuf_idx); // TODO: test stack!
}

// int
// md_alloc_bulk();

// int
// buf_alloc_bulk();

#endif /* _MBUF_H_ */
