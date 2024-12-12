#ifndef _VHOST_H_
#define _VHOST_H_

#include <stdint.h>
#include <string.h>

#include <pkt.h>

#include "perf.h"
#include "vioqueue.h"
#include "vio_hdr.h"

#include <aggregated_md.h>
#include <memcpy_md.h>

struct vhost_queue {
    struct mpools *host_mpools;
    struct vioqueue *vq;
};

static inline int
desc_is_avail(struct desc *desc)
{
    return (*((volatile int16_t *)&desc->flags) & AVAIL_FLAG);
}

static inline int
vhost_rx_batch(struct vioqueue *vq, struct mbuf_ptr mps[], uint32_t count)
{
    uint8_t *desc_addrs[count];
    struct vio_hdr *hdrs[count];
    uint64_t lens[count];
    struct desc_mbuf_idx buf_idxs[count];
    struct desc *avail_descs = &vq->descs[vq->last_avail_idx];
    struct desc *used_descs = &vq->descs[vq->last_used_idx];
    uint16_t i = 0;

    /* vhost_rx_batch_check */
    // if (unlikely(avail_idx & PACKED_BATCH_MASK))
    if (unlikely((vq->last_avail_idx + count) > vq->nentries)) {
        return -1;
    }

    for (i = 0; i < count; i++) {
        buf_idxs[i] = get_desc_mbuf_idx(&avail_descs[i]);
    }

    // for (i = 0; i < num; i++) {
    //     lens[i] = vq->descs[avail_idx + i].len;
    // }

    for (i = 0; i < count; i++)
        desc_addrs[i] = mbuf_mtod(vq->mpools, buf_idxs[i]);

    /* vhost_rx_batch_copy */
    for (i = 0; i < count; i++) {
        dpdk_prefetch0(desc_addrs[i]);
#ifdef VIO_HEADER
        hdrs[i] = (struct vio_hdr *)desc_addrs[i] - 1;
#endif
#if METADATA_SIZE == 0
        lens[i] = PKT_SIZE;
#else
        lens[i] = mps[i].md->pkt_len;
#endif
    }

#ifdef VIO_HEADER
    for (i = 0; i < count; i++)
        vhost_enqueue_offload(hdrs[i], mps[i].md);
#endif

    vq->last_avail_idx += count;
    vq->last_avail_idx &= (vq->nentries - 1);

    vhost_memcpy_md_rx(vq->mpools, buf_idxs, mps, count);

    for (i = 0; i < count; i++)
        memcpy(desc_addrs[i], mps[i].pkt, lens[i]);

    for (i = 0; i < count; i++) {
        used_descs[i].len = lens[i];
    }

    // set_used
    dpdk_atomic_thread_fence(__ATOMIC_RELEASE);
    for (i = 0; i < count; i++)
        used_descs[i].flags = USED_FLAG;

    vq->last_used_idx += count;
    vq->last_used_idx &= (vq->nentries - 1);
    return 0;
}

uint16_t
vhost_enqueue_burst(struct vioqueue *vq, struct mbuf_ptr mps[], uint32_t count)
{
    uint32_t pkt_idx = 0;

    if (count == 0)
        return 0;

    do {
        if (desc_is_avail(&vq->descs[(vq->last_avail_idx + count - 1) & (vq->nentries -1)])) {
            if (!vhost_rx_batch(vq, &mps[pkt_idx], count)) {
                pkt_idx += count;
                break;
            }
        }
        count >>= 1;
    } while (count > 0);

    return pkt_idx;
}

int
vhost_tx_batch(struct vioqueue *vq, struct mbuf_ptr mps[], uint32_t count)
{
    uint8_t *desc_addrs[count];
    struct vio_hdr *hdr;
    struct desc_mbuf_idx buf_idxs[count];
    struct desc *avail_descs = &vq->descs[vq->last_avail_idx];
    // struct desc *used_descs = &vq->descs[vq->last_used_idx];
    uint16_t i = 0;

    if (unlikely(vq->last_avail_idx + count > vq->nentries))
        return -1;

    for (i = 0; i < count; i++)
        buf_idxs[i] = get_desc_mbuf_idx(&avail_descs[i]);

#if METADATA_SIZE > 0
    for (i = 0; i < count; i++) {
        mps[i].md->pkt_len = avail_descs[i].len;
    }
#endif

    for (i = 0; i < count; i++)
        desc_addrs[i] = mbuf_mtod(vq->mpools, buf_idxs[i]);

    for (i = 0; i < count; i++)
        dpdk_prefetch0(desc_addrs[i]);

    for (i = 0; i < count; i++) {
#if METADATA_SIZE == 0
        memcpy(mps[i].pkt, desc_addrs[i], PKT_SIZE);
#else
        memcpy(mps[i].pkt, desc_addrs[i], mps[i].md->pkt_len);
#endif
    }
    
    vhost_memcpy_md_tx(vq->mpools, buf_idxs, mps, count);

    // count = recognize_mds_host_tx(vq, mps, count);
    // free_aggregated_md_shm(vq->mpools, buf_idxs, count);

#ifdef VIO_HEADER
    if (vq->is_offload) {
        for (i = 0; i < count; i++) {
            hdr = (struct vio_hdr *)desc_addrs[i] - 1;
            vhost_dequeue_offload(hdr);
        }
    }
#endif

    dpdk_atomic_thread_fence(__ATOMIC_RELEASE);
    for (i = 0; i < count; i++)
        avail_descs[i].flags = USED_FLAG;

    vq->last_avail_idx += count;
    vq->last_avail_idx &= (vq->nentries - 1);

    return 0;
}

uint16_t
vhost_dequeue_burst(struct vhost_queue *vhq, struct mbuf_ptr mps[], uint32_t count)
{
    uint32_t pkt_idx = 0;
    uint32_t batchsz = count;

    if (count == 0) {return 0;}

    for (uint32_t i = 0; i < count; i++) {
        reset_mbptr(&mps[i], mbuf_alloc(vhq->host_mpools), vhq->host_mpools);
    }
    alloc_aggregated_md_local(vhq->host_mpools, mps, count);

    do {
        if (desc_is_avail(&vhq->vq->descs[(vhq->vq->last_avail_idx + batchsz - 1) & (vhq->vq->nentries - 1)])) {
            if (!vhost_tx_batch(vhq->vq, &mps[pkt_idx], batchsz)) {
                pkt_idx += batchsz;
                break;
            }
        }
        batchsz >>= 1;
    } while (batchsz > 0);

    for (uint32_t i = pkt_idx; i < count; i++) {
        mbuf_free(vhq->host_mpools, mps[i].mbuf_idx.dmidx);
    }
    free_aggregated_md_local(vhq->host_mpools, mps, count, pkt_idx);
    free_md_bulk(&mps[pkt_idx], count - pkt_idx);

    return pkt_idx;
}


#endif /* _VHOST_H_ */
