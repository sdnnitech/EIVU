#ifndef _MBUF_H_
#define _MBUF_H_

#include <stdint.h>
#include <string.h>

#include <mbuf_core.h>

#include "mpools.h"

struct mbuf_idx {
    int32_t pad;
    int32_t buf_idx;
};

struct mbuf_ptr {
    struct mbuf_idx mbuf_idx;
    struct mpools *mpools;
#ifdef MDQUE
    struct desc *md;
#else
    struct metadata *md;
#endif
    uint8_t *pkt;
};

static inline struct metadata*
get_metadata(struct mpools *mpools, struct mbuf_idx *idx)
{
    return (struct metadata *)&((uint8_t *)mpools->buf_pool.pool)[idx->buf_idx * mpools->buf_pool.memobj_size];
}

static inline uint8_t*
mbuf_mtod_offset(struct mpools *mpools, struct mbuf_idx *idx, int offset)
{
    return (uint8_t *)&((uint8_t *)mpools->buf_pool.pool)[idx->buf_idx * mpools->buf_pool.memobj_size] + offset;
}

static inline uint8_t*
mbuf_mtod(struct mpools *mpools, struct mbuf_idx *idx)
{
    return mbuf_mtod_offset(mpools, idx, METADATA_SIZE + MBUF_HEADROOM_SIZE);
}

static inline void
mbuf_alloc(struct mpools *mpools, struct mbuf_idx *idx)
{
    struct metadata* md;
    idx->buf_idx = memobj_get_stack(&mpools->buf_pool);

    md = get_metadata(mpools, idx);
    reset_metadata(md);

    return;
}

static inline void
mbuf_free(struct mpools *mpools, struct mbuf_idx *idx)
{
    memobj_put_stack(&mpools->buf_pool, idx->buf_idx);
}

static inline void
reset_mbptr(struct mbuf_ptr *mbptr, struct mbuf_idx *idx, struct mpools *mpools)
{
    mbptr->mbuf_idx.buf_idx = idx->buf_idx;
    mbptr->md = get_metadata(mpools, idx);
    mbptr->pkt = mbuf_mtod(mpools, idx);
    mbptr->mpools = mpools;
}

#endif /* _MBUF_H_ */
