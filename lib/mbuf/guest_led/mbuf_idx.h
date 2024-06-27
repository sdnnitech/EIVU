#ifndef _MBUF_IDX_H_
#define _MBUF_IDX_H_

#include <stdint.h>
#include <string.h>

#include <mbuf_core.h>

struct mbuf_idx {
#if BUF_NUM < 32768
    int16_t pktbuf_idx;
#else
    int32_t pktbuf_idx;
#endif
};

struct mbuf_idx
init_midx(void)
{
    struct mbuf_idx midx;
    midx.pktbuf_idx = -1;
    return midx;
}

#endif /* _MBUF_IDX_H_ */
