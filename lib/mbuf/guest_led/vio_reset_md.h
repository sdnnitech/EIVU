#ifndef _VIO_RESET_MD_H_
#define _VIO_RESET_MD_H_

#include <mbuf.h>
#include <vioqueue.h>
#include <vio_hdr.h>

static inline void
vio_reset_md_rx(struct vioqueue *vq, struct mbuf_ptr mb_ptrs[], uint32_t len[], uint16_t num)
{
#if METADATA_SIZE > 0
    struct mbuf_ptr *rxmb;
    uint16_t i;

    for (i = 0; i < num; i++) {
        rxmb = &mb_ptrs[i];

        rxmb->md->pkt_len = len[i];
#if METADATA_SIZE > 4
        rxmb->md->port = vq->port_id;
#endif

#ifdef VIO_HEADER
        if (vq->is_offload)
            vio_rx_offload((struct vio_hdr *)rxmb->pkt - 1);
#endif
    }
#endif
}

#endif
