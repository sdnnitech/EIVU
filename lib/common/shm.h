#ifndef _SHM_H_
#define _SHM_H_

#include "vioqueue.h"
#include "mbuf.h"
#include <vio_hdr.h>

#include <stdbool.h>

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/stat.h>

#define SHM_NAME "multimbuf"
#define HUGEPAGE_PATH "/dev/hugepages/"
#define SHM_SIZE 4000000000ULL
#define GIB 1073741824ULL
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

struct shm {
    uint8_t *head;
    uint64_t pktbuf_memobjs_offset;
    uint64_t md_memobjs_offset;
    uint64_t rxd_offset;
    uint64_t txd_offset;
    uint64_t end_offset;
    uint64_t start_offset;
};

static inline void*
md_memobjs(struct shm *shm)
{
    return (void *)(shm->head + shm->md_memobjs_offset);
}

static inline void*
pktbuf_memobjs(struct shm *shm)
{
    return (void *)(shm->head + shm->pktbuf_memobjs_offset);
}

static inline struct desc*
rxd(struct shm *shm)
{
    return (struct desc*)(shm->head + shm->rxd_offset);
}

static inline struct desc*
txd(struct shm *shm)
{
    return (struct desc*)(shm->head + shm->txd_offset);
}

static inline volatile bool*
end_flag(struct shm *shm)
{
    return (volatile bool*)(shm->head + shm->end_offset);
}

static inline volatile bool*
start_flag(struct shm *shm)
{
    return (volatile bool*)(shm->head + shm->start_offset);
}

void
init_shm(struct shm *shm, uint8_t *head, size_t mdmpool_size, size_t pktmpool_size, int rxtxd_size)
{
    shm->head = head;
    shm->pktbuf_memobjs_offset = 0;
    shm->md_memobjs_offset = pktmpool_size;
    shm->rxd_offset = 2 * GIB;
    shm->txd_offset = 2 * GIB + rxtxd_size;
    shm->end_offset = 2 * GIB + 2 * rxtxd_size;
    shm->start_offset = 2 * GIB + 2 * rxtxd_size + sizeof(uint64_t);
}

void
initialized_shm_assert(int shm_fd, struct shm *shm, uint32_t vq_size)
{
    uint32_t i = 0;
    struct stat sb;

#ifdef VIO_HEADER
    assert(MBUF_HEADROOM_SIZE >= VIO_HEADER_SIZE);
#endif

    assert(fstat(shm_fd, &sb) == 0);
    //assert(sb.st_size == (off_t)SHM_SIZE);
    assert((uintptr_t)md_memobjs(shm) - (uintptr_t)pktbuf_memobjs(shm)==
        (size_t)PKTBUF_NUM * (size_t)MBUF_PKTBUF_SIZE);
    assert((size_t)PKTBUF_NUM * (size_t)MBUF_PKTBUF_SIZE + (size_t)MDBUF_NUM * (size_t)MDBUF_SIZE <= 2 * GIB);

    // whether descs are initialized at `flag_init` or not
    for (i = 0; i < vq_size; i++) {
        assert((rxd(shm)[i].flags & AVAIL_FLAG) == AVAIL_FLAG);
        assert(rxd(shm)[i].midx.pktbuf_idx >= 0);
    }
    for (i = 0; i < vq_size; i++) {
        assert((txd(shm)[i].flags & USED_FLAG) == USED_FLAG);
        assert(txd(shm)[i].midx.pktbuf_idx < 0);
    }
}


#endif /* _SHM_H_ */
