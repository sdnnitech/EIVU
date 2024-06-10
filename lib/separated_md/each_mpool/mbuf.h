#ifndef _MBUF_H_
#define _MBUF_H_

#include <stdint.h>
#include <string.h>

#include <mbuf_struct.h>

#include "mpools.h"

static inline struct metadata*
get_metadata(struct mpools *mpools, struct mbuf_idx *idx)
{
    return (struct metadata *)&((uint8_t *)mpools->md_pool.pool)[idx->md_idx * mpools->md_pool.memobj_size];
}

static inline uint8_t*
mbuf_mtod_offset(struct mpools *mpools, struct mbuf_idx *idx, int offset)
{
    return (uint8_t *)&((uint8_t *)mpools->pktbuf_pool.pool)[idx->pktbuf_idx * mpools->pktbuf_pool.memobj_size] + offset;
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
    idx->md_idx = memobj_get_stack(&mpools->md_pool);

    md = get_metadata(mpools, idx);
    reset_metadata(md);

    idx->pktbuf_idx = memobj_get_stack(&mpools->pktbuf_pool);

    return;
}

static inline void
mbuf_free(struct mpools *mpools, struct mbuf_idx *idx)
{
    memobj_put_stack(&mpools->md_pool, idx->md_idx);
    memobj_put_stack(&mpools->pktbuf_pool, idx->pktbuf_idx);
}

static inline void
reset_mbptr(struct mbuf_ptr *mbptr, struct mbuf_idx *idx, struct mpools *mpools)
{
    mbptr->mbuf_idx.md_idx = idx->md_idx;
    mbptr->mbuf_idx.pktbuf_idx = idx->pktbuf_idx;
    mbptr->md = get_metadata(mpools, idx);
    mbptr->pkt = mbuf_mtod(mpools, idx);
    mbptr->mpools = mpools;
}

#endif /* _MBUF_H_ */
