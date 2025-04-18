#ifndef _MBUF_H_
#define _MBUF_H_

#include <stdint.h>
#include <string.h>

#include <desc.h>

#include "mpools.h"

struct mbuf_ptr {
    struct mbuf_idx mbuf_idx;
    struct mpools *mpools;
    struct metadata *md;
    uint8_t *pkt;
};

static inline struct metadata*
refer_integrated_metadata(struct mpools *mpools, struct desc_mbuf_idx idx)
{
    return (struct metadata *)&((uint8_t *)mpools->pktbuf_pool.pool)[idx.pktbuf_idx * mpools->pktbuf_pool.memobj_size];
}

static inline struct desc_mbuf_idx
mbuf_alloc(struct mpools *mpools)
{
    struct desc_mbuf_idx idx;

    idx.pktbuf_idx = alloc_pktbuf(&mpools->pktbuf_pool);

    return idx;
}

static inline void
mbuf_free(struct mpools *mpools, struct desc_mbuf_idx dmidx)
{
    free_pktbuf(&mpools->pktbuf_pool, dmidx.pktbuf_idx);
}

static inline void
free_md_bulk(struct mbuf_ptr mb_ptrs[], uint16_t nb_tx)
{
    return;
}

static inline uint8_t*
mbuf_mtod_offset(struct mpools *mpools, struct desc_mbuf_idx idx, int offset)
{
    return (uint8_t *)&((uint8_t *)mpools->pktbuf_pool.pool)[idx.pktbuf_idx * mpools->pktbuf_pool.memobj_size] + offset;
}

static inline uint8_t*
mbuf_mtod(struct mpools *mpools, struct desc_mbuf_idx idx)
{
    return mbuf_mtod_offset(mpools, idx, MDBUF_SIZE + MBUF_HEADROOM_SIZE);
}

static inline void
reset_mbptr(struct mbuf_ptr *mbptr, struct desc_mbuf_idx idx, struct mpools *mpools)
{
    mbptr->mbuf_idx.dmidx.pktbuf_idx = idx.pktbuf_idx;
    mbptr->md = refer_integrated_metadata(mpools, idx);
    mbptr->pkt = mbuf_mtod(mpools, idx);
    mbptr->mpools = mpools;
}

#endif /* _MBUF_H_ */
