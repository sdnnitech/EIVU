#ifndef _VIOQUEUE_OPS_H_
#define _VIOQUEUE_OPS_H_

#include <vioqueue.h>

static inline void
get_desc_mbuf_idx(struct desc *desc, struct mbuf_idx *idx)
{
    idx->md_idx = desc->md_idx;
    idx->pktbuf_idx = desc->buf_idx;
}

static inline void
set_desc_mbuf_idx(struct desc *desc, struct mbuf_idx *idx)
{
    desc->md_idx = idx->md_idx;
    desc->buf_idx = idx->pktbuf_idx;
}

#endif /* _VIOQUEUE_OPS_H_ */
