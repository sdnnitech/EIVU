#ifndef _MBUF_STRUCT_H_
#define _MBUF_STRUCT_H_

#include <stdint.h>
#include <string.h>

#include <mbuf_core.h>

struct mbuf_idx {
    int32_t md_idx;
    int32_t pktbuf_idx;
};

struct mbuf_ptr {
    struct mbuf_idx mbuf_idx;
    struct mpools *mpools;
#ifdef MDQUE
    struct desc *md;
#else
    struct metadata *md;
#endif
    uint8_t *pkt;
};

#endif /* _MBUF_STRUCT_H_ */
