#ifndef _VHOST_H_
#define _VHOST_H_

#include <stdint.h>
#include <string.h>

#include "perf.h"
#include "vioqueue.h"
#include "mbuf.h"

static inline int
desc_is_avail(struct desc *desc)
{
    return (*((volatile int16_t *)&desc->flags) & AVAIL_FLAG);
}

static inline int16_t
vhost_rx_single(struct vioqueue *vq, struct mbuf_ptr *mbp)
{
    struct desc *avail_desc = &vq->descs[vq->last_avail_idx];
    struct desc *used_desc = &vq->descs[vq->last_used_idx];

    if (unlikely(!desc_is_avail(avail_desc))) {
        return -1;
    }
    vq->last_avail_idx++;
    vq->last_avail_idx &= (vq->nentries - 1);

    memcpy((uint8_t *)&vq->mpool->pool[avail_desc->buf_idx] + MBUF_HEADROOM_SIZE,
        (uint8_t *)&mbp->mpool->pool[mbp->mbuf_idx] + MBUF_HEADROOM_SIZE,
            mbp->md->pkt_len);

    used_desc->len = mbp->md->pkt_len;
    dpdk_atomic_thread_fence(__ATOMIC_RELEASE);
    used_desc->flags = USED_FLAG;
    vq->last_used_idx++;
    vq->last_used_idx &= (vq->nentries - 1);
    return 0;
}

#define PACKED_BATCH_SIZE (CACHE_LINE_SIZE / \
			    sizeof(struct desc))

static inline int
vhost_rx_batch(struct vioqueue *vq, struct mbuf_ptr mps[])
{
    uint64_t lens[MAX_BATCH_SIZE] = {0};
    uint32_t buf_idxs[MAX_BATCH_SIZE] = {0};
    struct desc *avail_descs = &vq->descs[vq->last_avail_idx];
    struct desc *used_descs = &vq->descs[vq->last_used_idx];
    uint16_t i = 0;

    /* vhost_rx_batch_check */
    // if (unlikely(avail_idx & PACKED_BATCH_MASK))
    if (unlikely((vq->last_avail_idx + PACKED_BATCH_SIZE) > vq->nentries)) {
        return -1;
    }
    
    for (i = 0; i < PACKED_BATCH_SIZE; i++) {
        if (unlikely(!desc_is_avail(&avail_descs[i]))) {
            return -1;
        }
    }

    for (i = 0; i < PACKED_BATCH_SIZE; i++) {
        buf_idxs[i] = avail_descs[i].buf_idx;
    }

    // for (i = 0; i < num; i++) {
    //     lens[i] = vq->descs[avail_idx + i].len;
    // }

    /* vhost_rx_batch_copy */
    for (i = 0; i < PACKED_BATCH_SIZE; i++) {
        lens[i] = mps[i].md->pkt_len;
    }

    vq->last_avail_idx += PACKED_BATCH_SIZE;
    vq->last_avail_idx &= (vq->nentries - 1);

    for (i = 0; i < PACKED_BATCH_SIZE; i++) {
        memcpy((uint8_t *)&vq->mpool->pool[buf_idxs[i]] + MBUF_HEADROOM_SIZE,
            (uint8_t *)&mps[i].mpool->pool[mps[i].mbuf_idx] + MBUF_HEADROOM_SIZE, 
                lens[i]);
    }

    for (i = 0; i < PACKED_BATCH_SIZE; i++) {
        used_descs[i].len = lens[i];
    }

    // set_used
    dpdk_atomic_thread_fence(__ATOMIC_RELEASE);
    for (i = 0; i < PACKED_BATCH_SIZE; i++) {
        used_descs[i].flags = USED_FLAG;
    }

    vq->last_used_idx += PACKED_BATCH_SIZE;
    vq->last_used_idx &= (vq->nentries - 1);
    return 0;
}

uint16_t
vhost_enqueue_burst(struct vioqueue *vq, struct mbuf_ptr mps[], uint32_t count)
{
    uint32_t pkt_idx = 0;

    if (count == 0) {return 0;}
    do {
        // prefetch if needed
        if (count - pkt_idx >= PACKED_BATCH_SIZE) {
            if (!vhost_rx_batch(vq, &mps[pkt_idx])) {
                pkt_idx += PACKED_BATCH_SIZE;
                continue;
            }
        }

        if (vhost_rx_single(vq, &mps[pkt_idx])) {
            break;
        }
        pkt_idx++;
    } while (pkt_idx < count);

    return pkt_idx;
}

// uint16_t
// vhost_dequeue_burst()
// {
//     uint32_t pkt_idx;
//     return pkt_idx;
// }


#endif /* _VHOST_H_ */
