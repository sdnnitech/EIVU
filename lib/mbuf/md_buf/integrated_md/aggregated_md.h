#ifndef _AGGREGATED_MD_H_
#define _AGGREGATED_MD_H_

#include <stdint.h>

#include <mbuf.h>
#include <md_get_put.h>
#include <vioqueue.h>
#include <vio_reset_md.h> // TBD: to be moved to vio-related file

static inline void
aggregate_md(struct mbuf_ptr mps[], uint32_t batchsz)
{
    return;
}

static inline void
free_aggregated_md_local(struct mpools *mpools, struct mbuf_ptr mps[], uint32_t batchsz, uint32_t start_idx) // for TX
{
    return;
}

static inline void
reset_mbptr_aggregated_md_local(struct mbuf_ptr *mbp, int32_t md_idx, uint32_t batch_md_idx)
{
    return;
}


/*
* `num` in `vioqueue_dequeue_burst_rx()` catch this return-value.
* for each TX at guest and host.
* TX at host will use it after packet copy.
*/
/*
static inline uint32_t
recognize_mds_guest(struct vioqueue *vq, struct mbuf_ptr mps[], uint32_t num) 
{
    return num;
}

static inline uint32_t
recognize_mds_host_tx(struct vioqueue *vq, struct mbuf_ptr mps[], uint32_t num) 
{
    return num;
}
*/

static inline void
alloc_aggregated_md_local(struct mpools *local_mpools, struct mbuf_ptr mps[], uint32_t num) 
{
#if METADATA_SIZE > 64
#ifdef GUEST_LED
    for (uint16_t i = 0; i < num; i++) {
        dpdk_prefetch0(mps[i].md);
    }
#endif
#endif
}

// TBD: to be moved to vio-realated file
static inline void
setup_metadata(struct mpools *local_mpools, struct mbuf_ptr mps[], uint32_t len[], uint32_t num, uint16_t port_id, bool is_offload)
{
    for (uint32_t i = 0; i < num; i++) {
        vio_reset_md_rx(&mps[i], len[i], port_id, is_offload);
    }
}

#endif
