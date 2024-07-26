#ifndef _MD_GET_PUT_H_
#define _MD_GET_PUT_H_

#include <memobj.h>
#include <ring.h>
#include <stack.h>

#include <stdio.h>
#include <stdlib.h>

static inline int32_t
alloc_md(struct memobj_pool *mpool)
{
    int32_t mobj_idx = dequeue(mpool->ring);
    if (unlikely(mobj_idx < 0)) {
        fprintf(stderr, "get_md: no available memobjs\n");
        exit(EXIT_FAILURE);
    }

    return mobj_idx;
}

#endif
