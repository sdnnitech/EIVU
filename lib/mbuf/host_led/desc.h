#ifndef _DESC_H_
#define _DESC_H_

#include <stdint.h>

#include <mbuf_core.h>

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
#ifdef MDQUE
    struct md_rest md;
#endif
};

#endif
