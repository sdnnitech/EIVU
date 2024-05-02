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
    uint8_t memobjs[BUF_NUM][METADATA_SIZE + DATAROOM_SIZE];
    struct desc desc_rx[VQ_ENTRY_NUM];
    struct desc desc_tx[VQ_ENTRY_NUM];
    volatile bool is_end;
    volatile bool is_start;
};

void
initialized_shm_assert(int shm_fd, struct shm *shm)
{
    int i = 0;
    struct stat sb;
    const size_t MEMOBJ_SIZE = METADATA_SIZE + DATAROOM_SIZE;

    assert(fstat(shm_fd, &sb) == 0);
    assert(sb.st_size == (off_t)SHM_SIZE);
    assert(offsetof(struct shm, desc_rx) == (size_t)BUF_NUM * (size_t)MEMOBJ_SIZE);

    // whether descs are initialized at `flag_init` or not
    for (i = 0; i < VQ_ENTRY_NUM; i++) {
        assert((shm->desc_rx[i].flags & AVAIL_FLAG) == AVAIL_FLAG);
        assert(shm->desc_rx[i].buf_idx >= 0);
    }
    for (i = 0; i < VQ_ENTRY_NUM; i++) {
        assert((shm->desc_tx[i].flags & USED_FLAG) == USED_FLAG);
        assert(shm->desc_tx[i].buf_idx < 0);
    }
}


#endif /* _SHM_H_ */
