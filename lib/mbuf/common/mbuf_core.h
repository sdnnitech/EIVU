#ifndef _MBUF_CORE_H_
#define _MBUF_CORE_H_

#include <stdint.h>
#include <string.h>

#include <memobj.h>
#include <perf.h>
#include <pktbuf_get_put.h>

#define BUF_NUM 36864
#define METADATA_TOTAL_SIZE 128
#define METADATA_RESERV_SIZE 48
#define MBUF_HEADROOM_SIZE_DEFAULT 128
#define MBUF_DATAROOM_SIZE_DEFAULT 2048

#ifndef METADATA_SIZE
#define METADATA_SIZE METADATA_TOTAL_SIZE
#endif

#ifndef MDBUF_SIZE
#define MDBUF_SIZE METADATA_SIZE
#endif

#ifndef MBUF_HEADROOM_SIZE
#define MBUF_HEADROOM_SIZE MBUF_HEADROOM_SIZE_DEFAULT
#endif

#ifndef MBUF_DATAROOM_SIZE
#define MBUF_DATAROOM_SIZE MBUF_DATAROOM_SIZE_DEFAULT
#endif

#define MBUF_PKTBUF_SIZE (MBUF_HEADROOM_SIZE + MBUF_DATAROOM_SIZE)

struct metadata {
#if METADATA_SIZE == 2
    uint16_t pkt_len;
    uint16_t pad;
#else
    uint32_t pkt_len;
#endif
    uint16_t port;
    uint16_t nb_segs;
    uint64_t ol_flags;
};

static inline void
reset_metadata(struct metadata *md)
{
#if METADATA_SIZE >= METADATA_TOTAL_SIZE - METADATA_RESERV_SIZE
    memset(md, 0, METADATA_TOTAL_SIZE - METADATA_RESERV_SIZE);
#else
    memset(md, 0, METADATA_SIZE);
#endif

#if METADATA_SIZE >= 8
    md->nb_segs = 1;
#endif
}

#endif /* _MBUF_CORE_H_ */
