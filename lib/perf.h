#ifndef _PERF_H_
#define _PERF_H_

// #define _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>

#include <assert.h>

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

pid_t gettid() {
	return syscall(SYS_gettid);
}

void bind_core(int coreid) {
	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);
	CPU_SET(coreid, &cpu_set);
	int result = sched_setaffinity(gettid(), sizeof(cpu_set_t), &cpu_set);
	assert(result == 0);
}

#endif /* _PERF_H_ */
