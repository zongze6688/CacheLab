/*
 * solver.h - API for Cache Lab Bonus: reverse-engineer a cache
 *
 * Addresses are 32 bits. A cache has 2^s sets, E lines per set and
 * 2^b-byte blocks: block offset = the low b bits, set index = the next s
 * bits, tag = the rest. Ranges: b in 3..7, s in 1..9, E one of
 * 1, 2, 3, 4, 5, 6, 8, 12, 16.
 */
#ifndef SOLVER_H
#define SOLVER_H
#include <stdint.h>

/* Replacement policies, see the README for the exact definitions.
   With E = 1 the policy is not graded. */
enum { POLICY_LRU = 0, POLICY_FIFO = 1, POLICY_RANDOM = 2 };

/* Load or store the byte at addr. Returns 1 on a hit, 0 on a miss. */
int cache_load(uint32_t addr);
int cache_store(uint32_t addr);
/* Invalidate every line and reset the replacement state. */
void cache_flush(void);
/*
 * report - Give your answer. b, s: block offset and set index bits;
 *     E: lines per set; policy: one of POLICY_* (ignored when E = 1);
 *     write_allocate: 1 if a store miss brings the block in.
 */
void report(int b, int s, int E, int policy, int write_allocate);

/* You write this; it is called once per mystery cache. */
void solve(void);
#endif
