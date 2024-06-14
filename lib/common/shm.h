#ifndef _SHM_H_
#define _SHM_H_

#include "vioqueue.h"
#include "mbuf.h"

#include <stdbool.h>

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/stat.h>

#define SHM_NAME "SIVU"
#define SHM_SIZE 5000000000
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

struct shm {
    uint8_t *head;
    int memobjs_offset;
    int rxd_offset;
    int txd_offset;
    int end_offset;
    int start_offset;
};

static inline void*
memobjs(struct shm *shm)
{
    return (void *)(shm->head + shm->memobjs_offset);
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
init_shm(struct shm *shm, uint8_t *head, int mpool_size, int rxtxd_size)
{
    shm->head = head;
    shm->memobjs_offset = 0;
    shm->rxd_offset = mpool_size;
    shm->txd_offset = mpool_size + rxtxd_size;
    shm->end_offset = mpool_size + 2 * rxtxd_size;
    shm->start_offset = mpool_size + 2 * rxtxd_size + sizeof(bool);
}

void
initialized_shm_assert(int shm_fd, struct shm *shm, uint32_t vq_size)
{
    uint32_t i = 0;
    struct stat sb;
    const size_t MEMOBJ_SIZE = METADATA_SIZE + DATAROOM_SIZE;

    assert(fstat(shm_fd, &sb) == 0);
    assert(sb.st_size == (off_t)SHM_SIZE);
    assert((uintptr_t)rxd(shm) - (uintptr_t)memobjs(shm) == (size_t)BUF_NUM * (size_t)MEMOBJ_SIZE);

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
