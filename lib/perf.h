#ifndef _PERF_H_
#define _PERF_H_

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define CACHE_LINE_SIZE 64

#define MAX_BATCH_SIZE 1024

static inline __attribute__((always_inline)) void
dpdk_atomic_thread_fence(int memorder)
{
	if (memorder == __ATOMIC_SEQ_CST)
		__asm__ volatile("lock addl $0, -128(%%rsp); " ::: "memory");
	else
		__atomic_thread_fence(memorder);
}

#endif /* _PERF_H_ */
