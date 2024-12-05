#ifndef _DESC_H_
#define _DESC_H_

#include <stdint.h>

#include <mbuf_core.h>
#include <mpools.h>

struct desc_mbuf_idx {
#if PKTBUF_NUM < 32768
    int16_t pktbuf_idx;
#else
    int32_t pktbuf_idx;
#endif
};

struct desc_mbuf_idx
init_midx(void)
{
    struct desc_mbuf_idx midx;
    midx.pktbuf_idx = -1;
    return midx;
}

struct desc {
#if DESC_SIZE == 4
    struct desc_mbuf_idx midx;
    int16_t flags;
#elif DESC_SIZE == 8
    struct desc_mbuf_idx midx;
    uint16_t len;
    int16_t flags;
#else
    struct desc_mbuf_idx midx;
    int32_t pad;
    uint32_t len;
    int16_t id;
    int16_t flags;
#endif
};

struct mbuf_idx {
#if MDBUF_NUM < 32768
    int16_t md_idx;
#else
    int32_t md_idx;
#endif
    struct desc_mbuf_idx dmidx;
};

static inline struct metadata*
refer_metadata(struct mpools *mpools, struct mbuf_idx idx)
{
    return (struct metadata *)&((uint8_t *)mpools->md_pool.pool)[idx.md_idx * mpools->md_pool.memobj_size];
}

#endif
