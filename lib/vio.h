#ifndef _VIO_H_
#define _VIO_H_


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

    if (likely(num > DESC_PER_CACHELINE)) {
        num -= num % DESC_PER_CACHELINE;
    }
    num = vioqueue_dequeue_burst_rx(vq, bidxs, len, num); // finish accessing entiries of descs

    for (i = 0; i < num; i++) {
        rxmb = &mb_ptrs[i];
        reset_mbptr(rxmb, bidxs[i], vq->mpool);
        rxmb->md->pkt_len = len[i];
        rxmb->md->port = vq->port_id;
    }

    for (i = 0; i < num; i++) {
        reavail_desc = &vq->descs[vq->last_avail_idx];
        reavail_desc->buf_idx = memobj_get_stack(vq->mpool);
        dpdk_atomic_thread_fence(__ATOMIC_RELEASE);
        reavail_desc->flags = AVAIL_FLAG;

        vq->last_avail_idx++;
        vq->last_avail_idx &= (vq->nentries - 1);
    }

    return num;
}

uint16_t
vio_xmit_pkts(struct vioqueue *vq, struct mbuf_ptr **mb_ptrs, uint16_t nb_pkts);

#endif /* _VIO_H_ */
