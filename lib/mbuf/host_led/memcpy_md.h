#ifndef _MEMCPY_MD_H_
#define _MEMCPY_MD_H_

#include <stdint.h>
#include <mbuf.h>

static inline void
vhost_memcpy_md_rx(struct mpools *mpools_guest, struct desc_mbuf_idx dmidxs[], struct mbuf_ptr mb_ptrs[], uint32_t count)
{
#ifndef MDQUE
    for (uint32_t i = 0; i < count; i++) {
        memcpy(refer_metadata(mpools_guest, dmidxs[i]), mb_ptrs[i].md, METADATA_SIZE);
    }
#endif
}

static inline void
vhost_memcpy_md_tx(struct mpools *mpools_guest, struct desc_mbuf_idx dmidxs[], struct mbuf_ptr mb_ptrs[], uint32_t count)
{
#ifndef MDQUE
    for (uint32_t i = 0; i < count; i++) {
        memcpy(mb_ptrs[i].md, refer_metadata(mpools_guest, dmidxs[i]), METADATA_SIZE);
    }
#endif
}

#endif
