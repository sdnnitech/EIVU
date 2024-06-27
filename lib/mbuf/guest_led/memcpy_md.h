#ifndef _MEMCPY_MD_H_
#define _MEMCPY_MD_H_

#include <stdint.h>
#include <mbuf.h>

static inline void
vhost_memcpy_md_rx(struct mpools *mpools_guest, struct desc_mbuf_idx dmidxs[], struct mbuf_ptr mb_ptrs[], uint32_t count)
{
    return;
}

static inline void
vhost_memcpy_md_tx(struct mpools *mpools_guest, struct desc_mbuf_idx dmidxs[], struct mbuf_ptr mb_ptrs[], uint32_t count)
{
    return;
}

#endif
