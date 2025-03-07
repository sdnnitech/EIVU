#ifndef _VIO_H_
#define _VIO_H_

#include <mbuf.h>
#include <aggregated_md.h>
#include <vio_reset_md.h>
#include <mdque.h>

#include "vio_hdr.h"
#include "perf.h"

static inline int
desc_is_used(struct desc *desc)
{
    return (*((volatile int16_t *)&desc->flags) & USED_FLAG);
}

static inline uint16_t
vioqueue_dequeue_burst_rx(struct vioqueue *vq, struct desc_mbuf_idx idxs[], uint32_t *len, uint16_t num)
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
        idxs[i] = get_desc_mbuf_idx(used_desc);

        vq->last_used_idx++;
        vq->last_used_idx &= (vq->nentries - 1);
    }

    return i;
}

#define DESC_PER_CACHELINE (CACHE_LINE_SIZE / sizeof(struct desc))
uint16_t
vio_recv_pkts(struct vioqueue *vq, struct mbuf_ptr mb_ptrs[], uint16_t nb_pkts)
{
    uint32_t len[nb_pkts];
    struct desc_mbuf_idx idxs[nb_pkts];
    uint16_t num = nb_pkts;
    uint16_t i = 0;

    if (likely(num > DESC_PER_CACHELINE)) {
        num -= num % DESC_PER_CACHELINE;
    }
    num = vioqueue_dequeue_burst_rx(vq, idxs, len, num);

    for (i = 0; i < num; i++) {
        reset_mbptr(&mb_ptrs[i], idxs[i], vq->mpools);
    }

    // num = recognize_mds_guest(vq, mb_ptrs, num);
    alloc_aggregated_md_local(vq->mpools, mb_ptrs, num);

    vio_reset_md_rx(vq, mb_ptrs, len, num);

    vioqueue_refill_desc_rx(vq, num);

    return num;
}

static inline void
virtio_xmit_cleanup(struct vioqueue *vq, uint16_t num)
{
    int nb = num;
    uint16_t used_idx = vq->last_used_idx;
    uint16_t free_cnt = 0;

    while (nb > 0) {
        if (vq->descs[used_idx].midx.pktbuf_idx >= 0) {
            mbuf_free(vq->mpools, get_desc_mbuf_idx(&vq->descs[used_idx]));
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
vioqueue_enqueue_burst_tx(struct vioqueue *vq, struct mbuf_ptr *mbp)
{
    struct desc *d;

#ifdef VIO_HEADER
    if (!vq->is_offload)
        vio_tx_clear_net_hdr((struct vio_hdr *)mbp->pkt - 1);
    else {
        fprintf(stderr, "vio_tx_offload: caused an unexpected behavior\n");
        exit(EXIT_FAILURE);
    }
#endif

    d = &vq->descs[vq->last_avail_idx];
    set_desc_mbuf_idx(d, mbp->mbuf_idx.dmidx);
    vioqueue_set_len_tx(d, mbp);

    vq->last_avail_idx++;
    vq->last_avail_idx &= (vq->nentries - 1);
    vq->vq_free_cnt--;

}

uint16_t
vio_xmit_pkts(struct vioqueue *vq, struct mbuf_ptr mb_ptrs[], uint16_t nb_pkts)
{
    uint16_t nb_tx = 0;
    uint16_t lai_shadow = vq->last_avail_idx;

    if (unlikely(nb_pkts < 1)) 
        return 0;

    if (!desc_is_used(&vq->descs[(vq->last_used_idx + nb_pkts - 1) & (vq->nentries - 1)]))
        return 0;

    if (nb_pkts > vq->vq_free_cnt)
        virtio_xmit_cleanup(vq, nb_pkts - vq->vq_free_cnt);
    if (nb_pkts > vq->vq_free_cnt)
        return 0; // or `nb_pkts = vq->vq_free_cnt`

    for (nb_tx = 0; nb_tx < nb_pkts; nb_tx++) {
        vioqueue_enqueue_burst_tx(vq, &mb_ptrs[nb_tx]);
    }

    dpdk_atomic_thread_fence(__ATOMIC_RELEASE);
    for (uint16_t i = 0; i < nb_tx; i++)
        vq->descs[lai_shadow++ & (vq->nentries - 1)].flags = AVAIL_FLAG;

    free_aggregated_md_local(vq->mpools, mb_ptrs, nb_tx, 0);

    free_md_bulk(mb_ptrs, nb_tx);

    return nb_tx;
}

#endif /* _VIO_H_ */
