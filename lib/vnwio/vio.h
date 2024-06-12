#ifndef _VIO_H_
#define _VIO_H_

#include <mbuf.h>
#include <vioqueue_ops.h>

#include "vio_hdr.h"
#include "perf.h"

static inline int
desc_is_used(struct desc *desc)
{
    return (*((volatile int16_t *)&desc->flags) & USED_FLAG);
}

static inline uint16_t
vioqueue_dequeue_burst_rx(struct vioqueue *vq, struct mbuf_idx idxs[], uint32_t *len, uint16_t num)
{
    struct desc *used_desc;
    uint16_t i = 0;

    do {
        if (desc_is_used(&vq->descs[(vq->last_used_idx + num - 1) & (vq->nentries - 1)]))
            break;
        num >>= 1;
    } while (num > 0);

    for (i = 0; i < num; i++) {
        used_desc = &vq->descs[vq->last_used_idx];

        len[i] = used_desc->len;
        get_desc_mbuf_idx(used_desc, &idxs[i]);

        dpdk_prefetch0(refer_metadata(vq->mpools, idxs[i]));

        vq->last_used_idx++;
        vq->last_used_idx &= (vq->nentries - 1);
    }

    return i;
}

static inline void
vio_reset_md_rx(struct metadata *md, struct vioqueue *vq, uint32_t len)
{
    md->pkt_len = len;
    md->port = vq->port_id;
}

static inline void
vioqueue_refill_desc_rx(struct vioqueue *vq)
{
    struct desc *reavail_desc = &vq->descs[vq->last_avail_idx];
    set_desc_mbuf_idx(reavail_desc, mbuf_alloc(vq->mpools));
}

#define DESC_PER_CACHELINE (CACHE_LINE_SIZE / sizeof(struct desc))
uint16_t
vio_recv_pkts(struct vioqueue *vq, struct mbuf_ptr mb_ptrs[], uint16_t nb_pkts)
{
    struct mbuf_ptr *rxmb;
    uint32_t len[nb_pkts];
    struct mbuf_idx idxs[nb_pkts];
    uint16_t num = nb_pkts;
    uint16_t i = 0;
    uint16_t lai_shadow = vq->last_avail_idx;
    int16_t f = AVAIL_FLAG;

    if (likely(num > DESC_PER_CACHELINE)) {
        num -= num % DESC_PER_CACHELINE;
    }
    num = vioqueue_dequeue_burst_rx(vq, idxs, len, num); // finish accessing entiries of descs

    for (i = 0; i < num; i++) {
        rxmb = &mb_ptrs[i];
        reset_mbptr(rxmb, idxs[i], vq->mpools);  // different between patterns
        vio_reset_md_rx(rxmb->md, vq, len[i]);

        if (vq->is_offload)
            vio_rx_offload((struct vio_hdr *)rxmb->pkt - 1);
    }

    for (i = 0; i < num; i++) {
        vioqueue_refill_desc_rx(vq);

        vq->last_avail_idx++;
        vq->last_avail_idx &= (vq->nentries - 1);
    }

    for (i = 0; i < num; i++)
        __atomic_store(&vq->descs[lai_shadow++ & (vq->nentries - 1)].flags, &f, __ATOMIC_RELEASE);

    return num;
}

static inline void
virtio_xmit_cleanup(struct vioqueue *vq, uint16_t num)
{
    struct mbuf_idx midx;
    int nb = num;
    uint16_t used_idx = vq->last_used_idx;
    uint16_t free_cnt = 0;

    while (nb > 0) {
        if (vq->descs[used_idx].buf_idx >= 0) {
            get_desc_mbuf_idx(&vq->descs[used_idx], &midx);
            mbuf_free(vq->mpools, midx);
        }
        used_idx++;
        used_idx &= (vq->nentries - 1);
        free_cnt++;
        nb--;
    }
    vq->last_used_idx = used_idx;
    vq->vq_free_cnt += free_cnt;
}

static inline void
vioqueue_enqueue_burst_tx(struct vioqueue *vq, struct mbuf_idx idx, uint32_t len)
{
    struct desc *d;

    if (!vq->is_offload)
        vio_tx_clear_net_hdr((struct vio_hdr *)mbuf_mtod(vq->mpools, idx) - 1);
    else {
        fprintf(stderr, "vio_tx_offload: caused an unexpected behavior\n");
        exit(EXIT_FAILURE);
    }

    d = &vq->descs[vq->last_avail_idx];
    set_desc_mbuf_idx(d, idx);
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
    int16_t f = AVAIL_FLAG;

    if (unlikely(nb_pkts < 1)) 
        return 0;

    if (!desc_is_used(&vq->descs[(vq->last_used_idx + nb_pkts - 1) & (vq->nentries - 1)]))
        return 0;

    if (nb_pkts > vq->vq_free_cnt)
        virtio_xmit_cleanup(vq, nb_pkts - vq->vq_free_cnt);
    if (nb_pkts > vq->vq_free_cnt)
        return 0; // or `nb_pkts = vq->vq_free_cnt`

    for (nb_tx = 0; nb_tx < nb_pkts; nb_tx++) {
        mbp = &mb_ptrs[nb_tx];
        vioqueue_enqueue_burst_tx(vq, mbp->mbuf_idx, mbp->md->pkt_len);
    }

    for (uint16_t i = 0; i < nb_tx; i++)
        __atomic_store(&vq->descs[lai_shadow++ & (vq->nentries - 1)].flags, &f, __ATOMIC_RELEASE);

    return nb_tx;
}

#endif /* _VIO_H_ */
