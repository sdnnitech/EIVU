#ifndef _MPOOLS_H_
#define _MPOOLS_H_

#include <stdio.h>
#include <stdlib.h>

#include <memobj.h>

void
init_mpools(struct mpools *mpools, void *memobjs, size_t memobj_size, const uint32_t memobj_num, const uint32_t cache_num)
{
    if (init_mpool(&mpools->pktbuf_pool, memobjs, memobj_size, memobj_num, cache_num) != 0) {
        fprintf(stderr, "init_mpool");
        exit(EXIT_FAILURE);
    }

    mpools->md_pool.pool = mpools->pktbuf_pool.pool;
    mpools->md_pool.memobj_size = mpools->pktbuf_pool.memobj_size;
}

void
fin_mpools(struct mpools *mpools)
{
    fin_mpool(&mpools->pktbuf_pool);
}

#endif /* _MPOOLS_H_ */
