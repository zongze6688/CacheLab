/*
 * perf.h - Shared trace and miss-count logic for test-trans and test-mul
 */
#ifndef CACHELAB_PERF_H
#define CACHELAB_PERF_H

/*
 * eval_one - Run "./<tracegen> -M M -N N -F fn" under valgrind, keep the
 *     memory accesses between the two markers in <trace_name>, and replay
 *     them on the reference simulator with cache parameters (s, E, b).
 *
 * Returns 1 and fills r[0..2] with hits, misses and evictions when the
 * function ran to completion and produced a correct result. Returns 0
 * otherwise, after printing the reason.
 */
int eval_one(const char *tracegen, int M, int N, int fn,
             const char *trace_name, int s, int E, int b, unsigned r[3]);

#endif /* CACHELAB_PERF_H */
