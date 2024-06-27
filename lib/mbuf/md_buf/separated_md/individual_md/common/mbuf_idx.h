#ifndef _MBUF_IDX_H_
#define _MBUF_IDX_H_

#include <stdint.h>
#include <string.h>

#include <desc.h>

struct mbuf_idx {
#if BUF_NUM < 32768
    int16_t md_idx;
#else
    int32_t md_idx;
#endif
    struct desc_mbuf_idx dmidx;
};

struct mbuf_ptr {
    struct mbuf_idx mbuf_idx;
    struct mpools *mpools;
    struct metadata *md;
    uint8_t *pkt;
};

#endif
