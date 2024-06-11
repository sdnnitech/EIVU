#ifndef _RING_H_
#define _RING_H_

#include <stdint.h>

struct ring_buf {
    int32_t *ptr;
    int32_t max;
    int32_t num;
    int32_t front;
    int32_t rear;
};

void*
init_ring(int32_t max, int32_t init_val)
{
    int32_t i = 0;
    struct ring_buf *ring = (struct ring_buf *)malloc(sizeof(struct ring_buf) + max * sizeof(int32_t));

    if (ring == NULL)
        return NULL;

    ring->ptr = (int32_t *)(ring + 1);
    for (i = 0; i < max; i++)
        ring->ptr[i] = init_val;
    
    ring->max = max;
    ring->num = 0;
    ring->front = 0;
    ring->rear = 0;

    return ring;
}

static inline int
enqueue(struct ring_buf *ring, int32_t data)
{
    if (ring->num == ring->max)
        return -1;

    ring->ptr[ring->rear++] = data;
    ring->num++;

    return 0;
}

static inline int32_t
dequeue(struct ring_buf *ring)
{
    if (ring->num <= 0)
        return -1;
    
    int32_t data = ring->ptr[ring->front++];
    ring->num--;

    return data;
}

#endif
