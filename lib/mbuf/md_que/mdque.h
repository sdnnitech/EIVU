#ifndef _MDQUE_H_
#define _MDQUE_H_

#include <vioqueue.h>

static inline void
vioqueue_refill_desc_rx(struct vioqueue *vq, uint16_t num)
{
    return;
}

static inline void
mdque_free_md(struct vioqueue *vq, uint16_t num)
{
    uint16_t i = 0;
    uint16_t lai_shadow = vq->last_avail_idx;

    for (i = 0; i < num; i++) {
        struct desc *reavail_desc = &vq->descs[vq->last_avail_idx];
        set_desc_mbuf_idx(reavail_desc, mbuf_alloc(vq->mpools));

        vq->last_avail_idx++;
        vq->last_avail_idx &= (vq->nentries - 1);
    }

    dpdk_atomic_thread_fence(__ATOMIC_RELEASE);
    for (i = 0; i < num; i++)
        vq->descs[lai_shadow++ & (vq->nentries - 1)].flags = AVAIL_FLAG;
}

static void
mdque_init_descs_rx(struct vioqueue *vq_rx)
{
    for (int32_t i = 0; i < vq_rx->nentries; i++) {
        vq_rx->descs[i].midx.md_idx = i;
    }
}

static inline void
vioqueue_set_len_tx(struct desc *d, struct mbuf_ptr *mbp)
{
    d->len = mbp->mbuf_idx.dmidx.md_idx;
}

static inline void
reset_mbptr(struct mbuf_ptr *mbptr, struct desc_mbuf_idx idx, struct mpools *mpools)
{
    mbptr->mbuf_idx.dmidx = idx;
    mbptr->desc_rx = &((struct vioqueue *)mpools->que_rx)->descs[idx.md_idx];
    mbptr->mbuf_idx.dmidx.md_idx = mbptr->desc_rx->len;
    mbptr->pkt = mbuf_mtod(mpools, mbptr->mbuf_idx.dmidx);
    mbptr->mpools = mpools;
}

#endif
