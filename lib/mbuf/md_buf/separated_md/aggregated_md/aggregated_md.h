#ifndef _AGGREGATED_MD_H_
#define _AGGREGATED_MD_H_

#include <stdint.h>

#include <mbuf.h>
#include <md_get_put.h>
#include <vioqueue.h>

#ifndef AGGREGATED_MD_NUM
#define AGGREGATED_MD_NUM 4
#endif

/*
static inline void
aggregate_md(struct mbuf_ptr mps[], uint32_t batchsz)
{
    uint32_t *landmark;
    for (uint32_t i = 0; i < batchsz; i += AGGREGATED_MD_NUM) {
        landmark = (uint32_t *)(mps[i].pkt - CACHE_LINE_SIZE);
        *landmark = AGGREGATED_MD_NUM;
    }
}
*/

static inline void
free_aggregated_md_local(struct mpools *mpools, struct mbuf_ptr mps[], uint32_t batchsz, uint32_t start_idx)
{
    uint32_t i = start_idx % AGGREGATED_MD_NUM == 0 ?
        start_idx : (start_idx / AGGREGATED_MD_NUM + 1) * AGGREGATED_MD_NUM;

    for (; i < batchsz; i += AGGREGATED_MD_NUM) {
        free_md(&mpools->md_pool, mps[i].mbuf_idx.md_idx);
    }
}

static inline void
reset_mbptr_aggregated_md(struct mbuf_ptr *mbp, int32_t md_idx, uint32_t batch_md_idx)
{
    mbp->mbuf_idx.md_idx = md_idx;
    mbp->batch_md_idx = batch_md_idx;
    mbp->md = (struct metadata *)((uint8_t *)refer_metadata(mbp->mpools, mbp->mbuf_idx) + batch_md_idx * METADATA_SIZE);
    reset_metadata(mbp->md);
}


/*
* `num` in `vioqueue_dequeue_burst_rx()` catch this return-value for each TX at guest and host.
* TX at host will use it after packet copy.
*/
/*
#define WAIT_COUNT_THRESH 1000000000UL
static inline uint32_t
recognize_mds_guest(struct vioqueue *vq, struct mbuf_ptr mps[], uint32_t num) 
{
    struct mbuf_ptr *mbp;
    int32_t md_idx;
    uint32_t i, j;
    uint32_t batchsz = 0;

    for (i = 0; i < num; i += batchsz) {
        md_idx = alloc_md(&vq->mpools->md_pool);
        batchsz = *(uint32_t *)(mps[i].pkt - CACHE_LINE_SIZE);
        for (j = 0; j < batchsz; j++) {
            if (i + j >= num) {
                uint64_t wait_counter = 0;
                while (!(*((volatile int16_t *)&vq->descs[vq->last_used_idx].flags) & USED_FLAG)) {
                    if (wait_counter++ >= WAIT_COUNT_THRESH) {
                        fprintf(stderr, "recognize_mds_guest: over threshold\n");
                        exit(EXIT_FAILURE);
                    }
                }

                reset_mbptr(&mps[i + j], vq->descs[vq->last_used_idx].midx, vq->mpools);

                vq->last_used_idx++;
                vq->last_used_idx &= (vq->nentries - 1);
                num++;
            }
           reset_mbptr_aggregated_md(&mps[i + j], md_idx, j);
        }
    }

    return num;
}

static inline uint32_t
recognize_mds_host_tx(struct vioqueue *vq, struct mbuf_ptr mps[], uint32_t num) 
{
    struct mbuf_ptr *mbp;
    int32_t md_idx;
    uint32_t i, j;
    uint32_t batchsz;

    for (i = 0; i < num; i += batchsz) {
        md_idx = alloc_md(&vq->mpools->md_pool);
        batchsz = *(uint32_t *)(mps[i].pkt - CACHE_LINE_SIZE);
        for (j = 0; j < batchsz; j++) {
            if (i + j >= num) {
                uint64_t wait_counter = 0;
                while (!(*((volatile int16_t *)&vq->descs[vq->last_avail_idx].flags) & AVAIL_FLAG)) {
                    if (wait_counter++ >= WAIT_COUNT_THRESH) {
                        fprintf(stderr, "recognize_mds_host: over threshold\n");
                        exit(EXIT_FAILURE);
                    }
                }

                memcpy(mbuf_mtod(mps[i + j].mpools, mps[i + j].mbuf_idx),
                    mbuf_mtod(vq->mpools, vq->descs[vq->last_avail_idx].midx),
                    vq->descs[vq->last_avail_idx].len);

                vq->last_avail_idx++;
                vq->last_avail_idx &= (vq->nentries - 1);
                num++;
            }
            reset_mbptr_aggregated_md(&mps[i + j], md_idx, j);
        }
    }

    return num;
}
*/

static inline void
alloc_aggregated_md_local(struct mpools *local_mpools, struct mbuf_ptr mps[], uint32_t num) 
{
    int32_t md_idx;
    uint32_t i, j;

    for (i = 0; i < num; i += AGGREGATED_MD_NUM) {
        md_idx = alloc_md(&local_mpools->md_pool);
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
