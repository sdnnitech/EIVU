#ifndef _MBUF_H_
#define _MBUF_H_

#include <stdint.h>
#include <string.h>

#include <mbuf_core.h>
#include <mbuf_idx.h>
#include <md_get_put.h>
#include <mpools.h>

static inline struct metadata*
refer_metadata(struct mpools *mpools, struct mbuf_idx idx)
{
#ifdef GUEST_LED
    return (struct metadata *)&((uint8_t *)mpools->md_pool.pool)[idx.md_idx * mpools->md_pool.memobj_size];
#elif defined HOST_LED
    return (struct metadata *)&((uint8_t *)mpools->md_pool.pool)[idx.dmidx.md_idx * mpools->md_pool.memobj_size];
#endif
}

static inline struct desc_mbuf_idx
mbuf_alloc(struct mpools *mpools)
{
    struct desc_mbuf_idx idx;

    idx.pktbuf_idx = alloc_pktbuf(&mpools->pktbuf_pool);

    return idx;
}

static inline void
mbuf_free(struct mpools *mpools, struct desc_mbuf_idx idx)
{
    free_pktbuf(&mpools->pktbuf_pool, idx.pktbuf_idx);
#ifdef HOST_LED
    free_md(&mpools->md_pool, idx.md_idx);
#endif
}

static inline void
free_md_bulk(struct mbuf_ptr mb_ptrs[], uint16_t nb_tx)
{
#ifdef GUEST_LED
    for (int16_t i = 0; i < nb_tx; i++)
        free_md(&mb_ptrs[i].mpools->md_pool, mb_ptrs[i].mbuf_idx.md_idx);
#endif
}

static inline uint8_t*
mbuf_mtod_offset(struct mpools *mpools, struct desc_mbuf_idx idx, int offset)
{
    return (uint8_t *)&((uint8_t *)mpools->pktbuf_pool.pool)[idx.pktbuf_idx * mpools->pktbuf_pool.memobj_size] + offset;
}

static inline uint8_t*
mbuf_mtod(struct mpools *mpools, struct desc_mbuf_idx idx)
{
    return mbuf_mtod_offset(mpools, idx, METADATA_SIZE + MBUF_HEADROOM_SIZE);
}

static inline void
reset_mbptr(struct mbuf_ptr *mbptr, struct desc_mbuf_idx idx, struct mpools *mpools)
{
    mbptr->mbuf_idx.dmidx = idx;

#if defined GUEST_LED
    mbptr->mbuf_idx.md_idx = alloc_md(&mpools->md_pool);
    mbptr->md = refer_metadata(mpools, mbptr->mbuf_idx);
#elif defined HOST_LED
    mbptr->md = refer_metadata(mpools, idx.md_idx);
#endif

    mbptr->pkt = mbuf_mtod(mpools, idx);
    mbptr->mpools = mpools;

    reset_metadata(mbptr->md);
}

#endif /* _MBUF_H_ */
