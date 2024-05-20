#ifndef _VIO_H_
#define _VIO_H_

#include "vio_hdr.h"
#include "vioqueue.h"
#include "mbuf.h"
#include "perf.h"

static inline int
desc_is_used(struct desc *desc)
{
    return (*((volatile int16_t *)&desc->flags) & USED_FLAG);
}

static inline uint16_t
vioqueue_dequeue_burst_rx(struct vioqueue *vq, int32_t *bidx, uint32_t *len, uint16_t num)
{
    struct desc *used_desc;
    uint16_t i = 0;

    for (i = 0; i < num; i++) {
        used_desc = &vq->descs[vq->last_used_idx];

        if (!desc_is_used(used_desc)) {
            return i;
        }

        len[i] = used_desc->len;
        bidx[i] = used_desc->buf_idx;

        dpdk_prefetch0(mbuf_mtod_offset(vq->mpool, used_desc->buf_idx, 0));

        vq->last_used_idx++;
        vq->last_used_idx &= (vq->nentries - 1);
    }

    return i;
}

#define DESC_PER_CACHELINE (CACHE_LINE_SIZE / sizeof(struct desc))
uint16_t
vio_recv_pkts(struct vioqueue *vq, struct mbuf_ptr mb_ptrs[], uint16_t nb_pkts)
{
    struct mbuf_ptr *rxmb;
    struct desc *reavail_desc;
    uint32_t len[MAX_BATCH_SIZE];
    int32_t bidxs[MAX_BATCH_SIZE];
    uint16_t num = nb_pkts;
    uint16_t i = 0;
    uint16_t lai_shadow = vq->last_avail_idx;

    if (likely(num > DESC_PER_CACHELINE)) {
        num -= num % DESC_PER_CACHELINE;
    }
    num = vioqueue_dequeue_burst_rx(vq, bidxs, len, num); // finish accessing entiries of descs

    for (i = 0; i < num; i++) {
        rxmb = &mb_ptrs[i];
        reset_mbptr(rxmb, bidxs[i], vq->mpool);

        rxmb->md->pkt_len = len[i];
        rxmb->md->port = vq->port_id;

        if (vq->is_offload)
            vio_rx_offload((struct vio_hdr *)rxmb->pkt - 1);
    }

    for (i = 0; i < num; i++) {
        reavail_desc = &vq->descs[vq->last_avail_idx];
        reavail_desc->buf_idx = mbuf_alloc(vq->mpool);

        vq->last_avail_idx++;
        vq->last_avail_idx &= (vq->nentries - 1);
    }

    dpdk_atomic_thread_fence(__ATOMIC_RELEASE);
    for (i = 0; i < num; i++)
        vq->descs[lai_shadow++ & (vq->nentries - 1)].flags = AVAIL_FLAG;

    return num;
}

static inline void
virtio_xmit_cleanup(struct vioqueue *vq, uint16_t num)
{
    int nb = num;
    uint16_t used_idx = vq->last_used_idx;
    uint16_t free_cnt = 0;

    while (nb > 0 && desc_is_used(&vq->descs[used_idx])) {
        if (vq->descs[used_idx].buf_idx >= 0)
            memobj_put_stack(vq->mpool, vq->descs[used_idx].buf_idx);
        used_idx++;
        used_idx &= (vq->nentries - 1);
        free_cnt++;
        nb--;
    }
    vq->last_used_idx = used_idx;
    vq->vq_free_cnt += free_cnt;
}

static inline void
vioqueue_enqueue_burst_tx(struct vioqueue *vq, int32_t bidx, uint32_t len)
{
    struct desc *d;

    if (!vq->is_offload)
        vio_tx_clear_net_hdr((struct vio_hdr *)mbuf_mtod(vq->mpool, bidx) - 1);
    else {
        fprintf(stderr, "vio_tx_offload: caused an unexpected behavior\n");
        exit(EXIT_FAILURE);
    }

    d = &vq->descs[vq->last_avail_idx];
    d->buf_idx = bidx;
    d->len = len;

    vq->last_avail_idx++;
    vq->last_avail_idx &= (vq->nentries - 1);
    vq->vq_free_cnt--;

}

uint16_t
vio_xmit_pkts(struct vioqueue *vq, struct mbuf_ptr mb_ptrs[], uint16_t nb_pkts)
{
    struct mbuf_ptr *mbp;
    uint16_t nb_tx = 0;
    uint16_t lai_shadow = vq->last_avail_idx;

    if (unlikely(nb_pkts < 1)) 
        return 0;

    if (nb_pkts > vq->vq_free_cnt)
        virtio_xmit_cleanup(vq, nb_pkts - vq->vq_free_cnt);
    if (nb_pkts > vq->vq_free_cnt)
        return 0; // or `nb_pkts = vq->vq_free_cnt`

    for (nb_tx = 0; nb_tx < nb_pkts; nb_tx++) {
        mbp = &mb_ptrs[nb_tx];
        vioqueue_enqueue_burst_tx(vq, mbp->mbuf_idx, mbp->md->pkt_len);
    }

    dpdk_atomic_thread_fence(__ATOMIC_RELEASE);
    for (uint16_t i = 0; i < nb_tx; i++)
        vq->descs[lai_shadow++ & (vq->nentries - 1)].flags = AVAIL_FLAG;

    return nb_tx;
}

#endif /* _VIO_H_ */
