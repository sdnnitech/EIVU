#ifndef _DESC_H_
#define _DESC_H_

#include <stdint.h>

#include <mbuf_idx.h>

struct desc {
#if DESC_SIZE == 4
    struct mbuf_idx midx;
    int16_t flags;
#elif DESC_SIZE == 8
    struct mbuf_idx midx;
    uint16_t len;
    int16_t flags;
#else
    struct mbuf_idx midx;
    int32_t pad;
    uint32_t len;
    int16_t id;
    int16_t flags;
#endif
};

#endif
