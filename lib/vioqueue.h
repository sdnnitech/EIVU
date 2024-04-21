#ifndef _VIOQUEUE_H_
#define _VIOQUEUE_H_

#include <stdint.h>

#include "mbuf.h"

#define AVAIL_FLAG (0b1 << 7)
#define USED_FLAG (0b1 << 15)

#define VQ_ENTRY_NUM 256 // can be optionalized

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
    uint16_t port_id;
    uint16_t nentries;
    uint16_t last_avail_idx;
    uint16_t last_used_idx;
    // uint16_t last_inflight_idx;
    struct desc *descs;
    struct memobj_pool *mpool;
};

void
init_vq(struct vioqueue *vq, uint16_t vq_entry_num, struct desc *descs, const uint16_t port_id, struct memobj_pool *mpool)
{
    vq->port_id = port_id;
    vq->nentries = vq_entry_num;
    vq->last_avail_idx = 0;
    vq->last_used_idx = 0;
    vq->descs = descs;
    vq->mpool = mpool;
}

#endif /* _VIOQUEUE_H_ */
