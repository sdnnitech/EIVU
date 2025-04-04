#ifndef _MBUF_CORE_H_
#define _MBUF_CORE_H_

#include <stdint.h>
#include <string.h>

#include <memobj.h>
#include <perf.h>
#include <pktbuf_get_put.h>

#define BUF_NUM_DEFAULT 36864
#define METADATA_TOTAL_SIZE 128
#define METADATA_RESERV_SIZE 48
#define MBUF_HEADROOM_SIZE_DEFAULT 128
#define MBUF_DATAROOM_SIZE_DEFAULT 2048

#ifndef MDBUF_NUM
#define MDBUF_NUM BUF_NUM_DEFAULT
#endif

#ifndef PKTBUF_NUM
#define PKTBUF_NUM BUF_NUM_DEFAULT
#endif

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

    uint8_t dummy[0];
} __attribute__((packed));

static inline void
reset_dummy_md(uint8_t *dummy)
{
#if 0
    memset(md->dummy, 0, METADATA_SIZE - 16);
#else
    {
        volatile uint64_t *p = (uint64_t*)dummy;
        const uint32_t num = (METADATA_SIZE - 16) / sizeof(*p);

        for (uint32_t i = 0; i < num; i++) {
            *p = 0;
            p++;
        }
    }
#endif
}

static inline void
reset_metadata(struct metadata *md)
{
#if METADATA_SIZE >= 8
    md->nb_segs = 1;

#if METADATA_SIZE >= 16
    md->ol_flags = 0;

#if METADATA_SIZE > 16
    reset_dummy_md(md->dummy);
#endif

#endif
#endif
}

#endif /* _MBUF_CORE_H_ */
