#ifndef _VIO_RESET_MD_H_
#define _VIO_RESET_MD_H_

#include <mbuf.h>
#include <vioqueue.h>
#include <vio_hdr.h>

static inline void
vio_reset_md_rx(struct mbuf_ptr *rxmb, uint32_t len, uint16_t port_id, bool is_offload)
{
#if METADATA_SIZE > 0

        rxmb->md->pkt_len = len;
#if METADATA_SIZE > 4
        rxmb->md->port = port_id;
#endif

        reset_metadata(rxmb->md);

#ifdef VIO_HEADER
        if (is_offload)
            vio_rx_offload((struct vio_hdr *)rxmb->pkt - 1);
#endif

#endif
}

#endif
