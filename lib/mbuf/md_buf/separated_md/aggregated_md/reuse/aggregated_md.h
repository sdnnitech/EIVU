#ifndef _AGGREGATED_MD_H_
#define _AGGREGATED_MD_H_

#include <stdint.h>

#include <mbuf.h>
#include <md_get_put.h>
#include <vioqueue.h>

#ifndef AGGREGATED_MD_NUM
#define AGGREGATED_MD_NUM 4
#endif

static inline void
free_aggregated_md_local(struct mpools *mpools, struct mbuf_ptr mps[], uint32_t batchsz, uint32_t start_idx)
{
    return;
}

static inline void
reset_mbptr_aggregated_md(struct mbuf_ptr *mbp, int32_t md_idx, uint32_t batch_md_idx)
{
    mbp->batch_md_idx = batch_md_idx;

#if defined GUEST_LED
    mbp->mbuf_idx.md_idx = md_idx;
    mbp->md = (struct metadata *)((uint8_t *)refer_metadata(mbp->mpools, mbp->mbuf_idx) + batch_md_idx * METADATA_SIZE);
#elif defined HOST_LED
    mbp->mbuf_idx.dmidx.md_idx = md_idx;
    mbp->md = (struct metadata *)((uint8_t *)refer_metadata(mbp->mpools, mbp->mbuf_idx.dmidx) + batch_md_idx * METADATA_SIZE);
#endif

    reset_metadata(mbp->md);
}

static inline void
alloc_aggregated_md_local(struct mpools *local_mpools, struct mbuf_ptr mps[], uint32_t num) 
{
    int32_t md_idx;
    uint32_t i, j;

    for (i = 0; i < num; i += AGGREGATED_MD_NUM) {
#if defined GUEST_LED
        if (unlikely(mps[i].mbuf_idx.md_idx < 0)) {
            md_idx = alloc_md(&local_mpools->md_pool);
        } else {
            md_idx = mps[i].mbuf_idx.md_idx;
        }
#elif defined HOST_LED
        if (unlikely(mps[i].mbuf_idx.dmidx.md_idx < 0)) {
            md_idx = alloc_md(&local_mpools->md_pool);
        } else {
            md_idx = mps[i].mbuf_idx.dmidx.md_idx;
        }
#endif

        for (j = 0; j < AGGREGATED_MD_NUM; j++) {
            if (i + j >= num) {
                i += num;
                break;
            }
            reset_mbptr_aggregated_md(&mps[i + j], md_idx, j);
        }
    }

    return;
}

#endif
