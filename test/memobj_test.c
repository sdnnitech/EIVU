#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "../lib/memobj.h"

int main(void)
{
    struct memobj_pool mpool;
    memobj *memobjs = NULL;

    /* Init. */
    memobjs = calloc(MEMOBJ_NUM, sizeof(memobj));
    if (memobjs == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    init_mpool(&mpool, memobjs, MEMOBJ_NUM, MEMOBJ_CACHE_NUM);

    /* Test */
#define BATCH_SZ 4
    int32_t mobj_idxs[BATCH_SZ];
    for (int i = 0; i < 16; i+=4) {
        printf("mpool.top=%d ", mpool.top);
        // get
        for (int j = 0; j < BATCH_SZ; j++) {
            mobj_idxs[j] = memobj_get_stack(&mpool);
            printf("mobj_idx=%d ", mobj_idxs[j]);
        }
        printf("mpool.top=%d ---- ", mpool.top);

        // put
        for (int j = 0; j < BATCH_SZ; j++)
            memobj_put_stack(&mpool, mobj_idxs[j]);
        printf("mpool.top=%d ", mpool.top);
        for (int j = 0; j < BATCH_SZ; j++)
            printf("mpool.cache[%d]=%d ", j, mpool.cache[j]);
        printf("\n");
    }

    /* Fin. */
    fin_mpool(&mpool);
    free(memobjs);
    return 0;
}
