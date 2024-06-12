#ifndef _DESC_H_
#define _DESC_H_

#include <stdint.h>

#include <mbuf.h>

struct desc {
#if DESC_SIZE == 4
    int16_t buf_idx;
    int16_t flags;
#elif DESC_SIZE == 8 && BUF_NUM < 32768
    int16_t buf_idx;
    int16_t flags;
    uint32_t len;
#elif DESC_SIZE == 8
    int32_t buf_idx;
    uint16_t len;
    int16_t flags;
#else
    int32_t md_idx;
    int32_t buf_idx;
    uint32_t len;
    int16_t id;
    int16_t flags;
#endif
};

#endif
