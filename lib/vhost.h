#ifndef _VHOST_H_
#define _VHOST_H_

#include <stdint.h>
#include <string.h>

#include "perf.h"
#include "vioqueue.h"
#include "vio_hdr.h"
#include "mbuf.h"

struct vhost_queue {
    struct memobj_pool *host_mpool;
    struct vioqueue *vq;
};

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
    uint8_t *desc_addr;

    if (unlikely(!desc_is_avail(avail_desc))) {
        return -1;
    }
    desc_addr = mbuf_mtod(vq->mpool, avail_desc->buf_idx);
    vq->last_avail_idx++;
    vq->last_avail_idx &= (vq->nentries - 1);

    vhost_enqueue_offload((struct vio_hdr *)desc_addr - 1, mbp->md);

    memcpy(desc_addr, mbp->pkt, mbp->md->pkt_len);

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
    uint8_t *desc_addrs[PACKED_BATCH_SIZE];
    struct vio_hdr *hdrs[PACKED_BATCH_SIZE];
    uint64_t lens[PACKED_BATCH_SIZE] = {0};
    uint32_t buf_idxs[PACKED_BATCH_SIZE] = {0};
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

    for (i = 0; i < PACKED_BATCH_SIZE; i++)
        desc_addrs[i] = mbuf_mtod(vq->mpool, buf_idxs[i]);

    /* vhost_rx_batch_copy */
    for (i = 0; i < PACKED_BATCH_SIZE; i++) {
        dpdk_prefetch0(desc_addrs[i]);
        hdrs[i] = (struct vio_hdr *)desc_addrs[i] - 1;
        lens[i] = mps[i].md->pkt_len;
    }

    for (i = 0; i < PACKED_BATCH_SIZE; i++)
        vhost_enqueue_offload(hdrs[i], mps[i].md);

    vq->last_avail_idx += PACKED_BATCH_SIZE;
    vq->last_avail_idx &= (vq->nentries - 1);

    for (i = 0; i < PACKED_BATCH_SIZE; i++)
        memcpy(desc_addrs[i], mps[i].pkt, lens[i]);

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
        dpdk_prefetch0(&vq->descs[vq->last_avail_idx]);

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

static inline int
vhost_tx_single(struct vioqueue *vq, struct mbuf_ptr *mbp)
{
    struct desc *avail_desc = &vq->descs[vq->last_avail_idx];
    // struct desc *used_desc = &vq->descs[vq->last_used_idx];
    uint8_t *desc_addr;

    if (unlikely(!desc_is_avail(avail_desc))) {
        return -1;
    }
    mbp->md->pkt_len = avail_desc->len;

    desc_addr = mbuf_mtod(vq->mpool, avail_desc->buf_idx);

    if (vq->is_offload)
        vhost_dequeue_offload((struct vio_hdr *)desc_addr - 1);

    memcpy(mbuf_mtod(mbp->mpool, mbp->mbuf_idx), desc_addr, avail_desc->len);

    dpdk_atomic_thread_fence(__ATOMIC_RELEASE);
    // used_desc->flags = USED_FLAG;
    // vq->last_used_idx++;
    // vq->last_used_idx &= (vq->nentries - 1);

    avail_desc->flags = USED_FLAG;
    vq->last_avail_idx++;
    vq->last_avail_idx &= (vq->nentries - 1);

    return 0;
}

int
vhost_tx_batch(struct vioqueue *vq, struct mbuf_ptr mps[])
{
    uint8_t *desc_addrs[PACKED_BATCH_SIZE];
    struct vio_hdr *hdr;
    uint64_t lens[PACKED_BATCH_SIZE] = {0};
    uint32_t buf_idxs[PACKED_BATCH_SIZE] = {0};
    struct desc *avail_descs = &vq->descs[vq->last_avail_idx];
    // struct desc *used_descs = &vq->descs[vq->last_used_idx];
    uint16_t i = 0;

    if (unlikely(vq->last_avail_idx + PACKED_BATCH_SIZE > vq->nentries))
        return -1;

    for (i = 0; i < PACKED_BATCH_SIZE; i++) {
        if (unlikely(!desc_is_avail(&avail_descs[i])))
            return -1;
    }

    for (i = 0; i < PACKED_BATCH_SIZE; i++)
        buf_idxs[i] = avail_descs[i].buf_idx;

    for (i = 0; i < PACKED_BATCH_SIZE; i++)
        lens[i] = avail_descs[i].len;

    for (i = 0; i < PACKED_BATCH_SIZE; i++)
        desc_addrs[i] = mbuf_mtod(vq->mpool, buf_idxs[i]);

    for (i = 0; i < PACKED_BATCH_SIZE; i++)
        dpdk_prefetch0(desc_addrs[i]);

    for (i = 0; i < PACKED_BATCH_SIZE; i++)
        memcpy(mbuf_mtod(mps[i].mpool, mps[i].mbuf_idx), desc_addrs[i], lens[i]);

    if (vq->is_offload) {
        for (i = 0; i < PACKED_BATCH_SIZE; i++) {
            hdr = (struct vio_hdr *)desc_addrs[i] - 1;
            vhost_dequeue_offload(hdr);
        }
    }

    dpdk_atomic_thread_fence(__ATOMIC_RELEASE);
    for (i = 0; i < PACKED_BATCH_SIZE; i++)
        avail_descs[i].flags = USED_FLAG;

    vq->last_avail_idx += PACKED_BATCH_SIZE;
    vq->last_avail_idx &= (vq->nentries - 1);

    return 0;
}

uint16_t
vhost_dequeue_burst(struct vhost_queue *vhq, struct mbuf_ptr mps[], uint32_t count)
{
    uint32_t pkt_idx = 0;

    if (count == 0) {return 0;}

    for (uint32_t i = 0; i < count; i++) {
        reset_mbptr(&mps[i], 0, mbuf_alloc(vhq->host_mpool), vhq->host_mpool);
    }
    do {
        dpdk_prefetch0(&vhq->vq->descs[vhq->vq->last_avail_idx]);

        if (count - pkt_idx >= PACKED_BATCH_SIZE) {
            if (!vhost_tx_batch(vhq->vq, &mps[pkt_idx])) {
                pkt_idx += PACKED_BATCH_SIZE;
                continue;
            }
        }

        if (vhost_tx_single(vhq->vq, &mps[pkt_idx])) {
            break;
        }
        pkt_idx++;
    } while (pkt_idx < count);

    for (uint32_t i = pkt_idx; i < count; i++) {
        mbuf_free(vhq->host_mpool, &mps[i]);
    }

    return pkt_idx;
}


#endif /* _VHOST_H_ */
