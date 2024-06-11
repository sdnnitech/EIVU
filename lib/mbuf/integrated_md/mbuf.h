#ifndef _MBUF_H_
#define _MBUF_H_

#include <stdint.h>
#include <string.h>

#include <mbuf_core.h>

#include "mpools.h"

static inline void
reset_mbptr(struct mbuf_ptr *mbptr, struct mbuf_idx *idx, struct mpools *mpools)
{
    mbptr->mbuf_idx.pktbuf_idx = idx->pktbuf_idx;
    mbptr->md = refer_metadata(mpools, idx);
    mbptr->pkt = mbuf_mtod(mpools, idx);
    mbptr->mpools = mpools;
}

#endif /* _MBUF_H_ */
