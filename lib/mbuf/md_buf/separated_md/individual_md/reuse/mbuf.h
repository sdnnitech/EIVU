#ifndef _MBUF_H_
#define _MBUF_H_

#include <stdint.h>
#include <string.h>

#include <mbuf_core.h>
#include <mbuf_idx.h>

#include "md_get_put.h"
#include "mpools.h"

static inline struct metadata*
refer_metadata(struct mpools *mpools, struct mbuf_idx idx)
{
    return (struct metadata *)&((uint8_t *)mpools->md_pool.pool)[idx.md_idx * mpools->md_pool.memobj_size];
}

static inline struct mbuf_idx
mbuf_alloc(struct mpools *mpools)
{
    struct mbuf_idx idx;

    idx.pktbuf_idx = alloc_pktbuf(&mpools->pktbuf_pool);

    return idx;
}

static inline void
mbuf_free(struct mpools *mpools, struct mbuf_idx idx)
{
    free_pktbuf(&mpools->pktbuf_pool, idx.pktbuf_idx);
}

static inline uint8_t*
mbuf_mtod_offset(struct mpools *mpools, struct mbuf_idx idx, int offset)
{
    return (uint8_t *)&((uint8_t *)mpools->pktbuf_pool.pool)[idx.pktbuf_idx * mpools->pktbuf_pool.memobj_size] + offset;
}

static inline uint8_t*
mbuf_mtod(struct mpools *mpools, struct mbuf_idx idx)
{
    return mbuf_mtod_offset(mpools, idx, METADATA_SIZE + MBUF_HEADROOM_SIZE);
}

static inline void
reset_mbptr(struct mbuf_ptr *mbptr, struct mbuf_idx idx, struct mpools *mpools)
{
    if (unlikely(mbptr->mbuf_idx.md_idx < 0)) {
        mbptr->mbuf_idx.md_idx = alloc_md(&mpools->md_pool);
    }
    mbptr->mbuf_idx.pktbuf_idx = idx.pktbuf_idx;
    mbptr->md = refer_metadata(mpools, mbptr->mbuf_idx);
    mbptr->pkt = mbuf_mtod(mpools, mbptr->mbuf_idx);
    mbptr->mpools = mpools;

    reset_metadata(refer_metadata(mpools, mbptr->mbuf_idx));
}

#endif /* _MBUF_H_ */
