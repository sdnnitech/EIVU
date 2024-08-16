#ifndef _DESC_H_
#define _DESC_H_

#include <stdint.h>

#include <mbuf_core.h>
#include <mpools.h>

struct desc_mbuf_idx {
#if BUF_NUM < 32768
    int16_t md_idx;
    int16_t buf_idx;
#else
    int32_t md_idx;
    int32_t pktbuf_idx;
#endif
};

struct desc_mbuf_idx
init_midx(void)
{
    struct desc_mbuf_idx midx;
    midx.pktbuf_idx = -1;
    midx.md_idx = -1;
    return midx;
}

struct md_rest {
    uint16_t nb_segs;
    uint64_t ol_flags;
};

struct desc {
#if DESC_SIZE == 8 && BUF_NUM < 32768
    struct desc_mbuf_idx midx;
    uint16_t len;
    int16_t flags;
#else
    struct desc_mbuf_idx midx;
    uint32_t len;
    int16_t id;
    int16_t flags;
#endif
#if 0
    struct md_rest md;
#endif
};

struct mbuf_idx {
    struct desc_mbuf_idx dmidx;
};

/* Both host and guest use this func to refer to buffs in shm. */
static inline struct metadata*
refer_metadata(struct mpools *mpools, struct desc_mbuf_idx idx)
{
#ifdef GUEST_INTEGRATED_MD
    return (struct metadata *)&((uint8_t *)mpools->pktbuf_pool.pool)[idx.pktbuf_idx * mpools->pktbuf_pool.memobj_size];
#else
    return (struct metadata *)&((uint8_t *)mpools->md_pool.pool)[idx.md_idx * mpools->md_pool.memobj_size];
#endif
}

#endif
