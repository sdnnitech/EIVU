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
    memobj memobjs[MEMOBJ_NUM];
    struct desc desc_rx[VQ_ENTRY_NUM];
    struct desc desc_tx[VQ_ENTRY_NUM];
};

void
initialized_shm_assert(int shm_fd, struct shm *shm, int16_t flag_init)
{
    int i = 0;
    int j = 0;
    struct stat sb;

    assert(fstat(shm_fd, &sb) == 0);
    assert(sb.st_size == (off_t)SHM_SIZE);
    assert(offsetof(struct shm, desc_rx) == (size_t)MEMOBJ_NUM * (size_t)MEMOBJ_SIZE);
    for (i = 0; i < MEMOBJ_NUM; i++) {
        for (j = 0; j < MEMOBJ_SIZE; j++) {
            assert(shm->memobjs[i][j] == 0); // whether bufs are initialized at 0 or not
        }
    }

    // whether descs are initialized at `flag_init` or not
    for (i = 0; i < VQ_ENTRY_NUM; i++) {
        assert(shm->desc_rx[i].flags == flag_init);
    }
    for (i = 0; i < VQ_ENTRY_NUM; i++) {
        assert(shm->desc_tx[i].flags == flag_init);
    }
}


#endif /* _SHM_H_ */
