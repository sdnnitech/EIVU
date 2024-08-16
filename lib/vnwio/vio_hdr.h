#ifndef _VIOHDR_H_
#define _VIOHDR_H_

#include <stdio.h>
#include <stdlib.h>

#include "mbuf_core.h"

#define VIO_HEADER_SIZE 12

struct vio_hdr {
    uint8_t flags;
    uint8_t gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
    uint16_t num_buffers;
};

/* DPDK's solution for avoiding RFO */
#define ASSIGN_UNLESS_EQUAL(var, val) do {     \
       if ((var) != (val))                     \
               (var) = (val);                  \
} while (0)

static inline void
vio_tx_clear_net_hdr(struct vio_hdr *vhdr)
{
    ASSIGN_UNLESS_EQUAL(vhdr->flags, 0);
    ASSIGN_UNLESS_EQUAL(vhdr->gso_type, 0);
    ASSIGN_UNLESS_EQUAL(vhdr->hdr_len, 0);
    ASSIGN_UNLESS_EQUAL(vhdr->gso_size, 0);
    ASSIGN_UNLESS_EQUAL(vhdr->csum_start, 0);
    ASSIGN_UNLESS_EQUAL(vhdr->csum_offset, 0);
    ASSIGN_UNLESS_EQUAL(vhdr->num_buffers, 0);
}

static inline void
vhost_enqueue_offload(struct vio_hdr *vhdr, struct metadata *md)
{
    uint64_t csum_l4 = md->ol_flags & (3ULL << 52);
    if (csum_l4) {
        fprintf(stderr, "vhost_enqueue_offload: caused an unexpected behavior\n");
        exit(EXIT_FAILURE);
    } else {
        vio_tx_clear_net_hdr(vhdr);
    }
}

static inline void
vhost_dequeue_offload(struct vio_hdr *vhdr)
{
    if (vhdr->flags == 0)
        return;

    fprintf(stderr, "vhost_dequeue_offload: caused an unexpected behavior\n");
    exit(EXIT_FAILURE);
}

static inline void
vio_rx_offload(struct vio_hdr *vhdr)
{
    if (vhdr->flags == 0)
        return;

    fprintf(stderr, "vio_rx_offload: caused an unexpected behavior\n");
    exit(EXIT_FAILURE);
}

#endif
