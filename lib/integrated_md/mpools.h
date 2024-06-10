#ifndef _MPOOLS_H_
#define _MPOOLS_H_

#include <stdio.h>
#include <stdlib.h>

#include <memobj.h>

struct mpools {
    struct memobj_pool buf_pool;
};

void
init_mpools(struct mpools *mpools, void *memobjs, size_t memobj_size, const uint32_t memobj_num, const uint32_t cache_num)
{
    if (init_mpool(&mpools->buf_pool, memobjs, memobj_size, memobj_num, cache_num) != 0) {
        fprintf(stderr, "init_mpool");
        exit(EXIT_FAILURE);
    }
}

void
fin_mpools(struct mpools *mpools)
{
    fin_mpool(&mpools->buf_pool);
}

#endif /* _MPOOLS_H_ */
