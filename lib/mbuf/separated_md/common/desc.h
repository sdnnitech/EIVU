#ifndef _DESC_H_
#define _DESC_H_

#include <stdint.h>

#include <mbuf.h>

#ifdef MDQUE
struct md_rest {
    uint16_t port;
};
#endif

struct desc {
#if DESC_SIZE == 8 && BUF_NUM < 32768
    struct mbuf_idx midx;
    uint16_t len;
    int16_t flags;
#else
    struct mbuf_idx midx;
    uint32_t len;
    int16_t id;
    int16_t flags;
#endif
#ifdef MDQUE
    struct md_rest md;
#endif
};

#endif
