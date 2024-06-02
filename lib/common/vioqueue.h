#ifndef _VIOQUEUE_H_
#define _VIOQUEUE_H_

#include <stdint.h>

#include "mbuf.h"

#define AVAIL_FLAG (0b1 << 7)
#define USED_FLAG (0b1 << 15)

#ifdef MDQUE
struct md_rest {
    uint16_t port;
};
#endif

struct desc {
    int32_t buf_idx;
    int32_t md_idx;
    uint32_t len;
    int16_t id;
    int16_t flags;
#ifdef MDQUE
    struct md_rest md;
#endif
};

struct vioqueue {
    uint16_t last_avail_idx;
    uint16_t last_used_idx;
    uint16_t nentries;
    uint16_t vq_free_cnt;
    // uint16_t last_inflight_idx;
    struct desc *descs;
    struct memobj_pool *mpool;
    bool is_offload;
    uint16_t port_id;
};

void
init_vq(struct vioqueue *vq, uint16_t vq_entry_num, struct desc *descs, const uint16_t port_id, struct memobj_pool *mpool)
{
    vq->is_offload = false;
    vq->port_id = port_id;
    vq->nentries = vq_entry_num;
    vq->last_avail_idx = 0;
    vq->last_used_idx = 0;
    vq->descs = descs;
    vq->mpool = mpool;
    vq->vq_free_cnt = 0;
}

#endif /* _VIOQUEUE_H_ */
