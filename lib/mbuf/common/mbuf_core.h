#ifndef _MBUF_CORE_H_
#define _MBUF_CORE_H_

#include <stdint.h>
#include <string.h>

#include <memobj.h>
#include <pktbuf_get_put.h>
#include <md_get_put.h>

#define BUF_NUM 163456
#define METADATA_SIZE 128
#define MBUF_HEADROOM_SIZE 128
#define DATAROOM_SIZE (MBUF_HEADROOM_SIZE + 2048)

struct mbuf_idx {
    int32_t md_idx;
    int32_t pktbuf_idx;
};

struct metadata {
    uint32_t pkt_len;
    uint16_t port;
    uint16_t nb_segs;
    uint64_t ol_flags;
};

struct mbuf_ptr {
    struct mbuf_idx mbuf_idx;
    struct mpools *mpools;
#ifdef MDQUE
    struct desc *md;
#else
    struct metadata *md;
#endif
    uint8_t *pkt;
};

static inline void
reset_metadata(struct metadata *md)
{
    memset(md, 0, CACHE_LINE_SIZE + 8 + 8);
    md->nb_segs = 1;
}

static inline struct metadata*
refer_metadata(struct mpools *mpools, struct mbuf_idx *idx)
{
    return (struct metadata *)&((uint8_t *)mpools->pktbuf_pool.pool)[idx->pktbuf_idx * mpools->pktbuf_pool.memobj_size];
}

static inline void
mbuf_alloc(struct mpools *mpools, struct mbuf_idx *idx)
{
    struct metadata* md;

    idx->pktbuf_idx = alloc_pktbuf(&mpools->pktbuf_pool);
    idx->md_idx = alloc_md(&mpools->md_pool);

    md = refer_metadata(mpools, idx);
    reset_metadata(md);

    return;
}

static inline void
mbuf_free(struct mpools *mpools, struct mbuf_idx *idx)
{
    free_md(&mpools->md_pool, idx->md_idx);
    free_pktbuf(&mpools->pktbuf_pool, idx->pktbuf_idx);
}

static inline uint8_t*
mbuf_mtod_offset(struct mpools *mpools, struct mbuf_idx *idx, int offset)
{
    return (uint8_t *)&((uint8_t *)mpools->pktbuf_pool.pool)[idx->pktbuf_idx * mpools->pktbuf_pool.memobj_size] + offset;
}

static inline uint8_t*
mbuf_mtod(struct mpools *mpools, struct mbuf_idx *idx)
{
    return mbuf_mtod_offset(mpools, idx, METADATA_SIZE + MBUF_HEADROOM_SIZE);
}

#endif /* _MBUF_CORE_H_ */
