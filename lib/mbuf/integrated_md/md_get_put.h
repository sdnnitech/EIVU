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
    return -1;
}

static inline int
free_md(struct memobj_pool *mpool, int32_t mobj_idx)
{
    return 0;
}

#endif
