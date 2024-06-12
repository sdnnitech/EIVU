#ifndef _MBUF_CORE_H_
#define _MBUF_CORE_H_

#include <stdint.h>
#include <string.h>

#include <memobj.h>
#include <pktbuf_get_put.h>

#define BUF_NUM 163456
#define METADATA_SIZE 128
#define MBUF_HEADROOM_SIZE 128
#define DATAROOM_SIZE (MBUF_HEADROOM_SIZE + 2048)

struct metadata {
    uint32_t pkt_len;
    uint16_t port;
    uint16_t nb_segs;
    uint64_t ol_flags;
};

static inline void
reset_metadata(struct metadata *md)
{
    memset(md, 0, CACHE_LINE_SIZE + 8 + 8);
    md->nb_segs = 1;
}

#endif /* _MBUF_CORE_H_ */
