#ifndef _STACK_H_
#define _STACK_H_

#include <stdint.h>
#include <stdlib.h>

struct stack {
    int32_t *ptr;
    int32_t max;
    int32_t top;
};

void*
init_stack(int32_t max, int32_t init_val)
{
    int32_t i = 0;
    struct stack *stk = (struct stack *)malloc(sizeof(struct stack) + max * sizeof(int32_t));

    if (stk == NULL)
        return NULL;

    stk->ptr = (int32_t *)(stk + 1);
    for (i = 0; i < max; i++)
        stk->ptr[i] = init_val;

    stk->max = max;
    stk->top = -1;

    return stk;
}

static inline int
push(struct stack *stk, int32_t data)
{
    if (stk->max == 0 || stk->top == stk->max - 1)
        return -1;

    stk->ptr[++stk->top] = data;
    return 0;
}

static inline int32_t
pop(struct stack *stk)
{
    if (stk->top == -1 || stk->max == 0)
        return -1;

    int32_t data = stk->ptr[stk->top--];
    return data;
}

#endif
