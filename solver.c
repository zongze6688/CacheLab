/*
 * solver.c - Starter for Cache Lab Bonus.
 * solve() is called once per mystery cache. See solver.h and README.md.
 */
#include "solver.h"

void solve(void)
{
    int b = 0;

    /* Load block 0. The first power-of-two offset that misses is its size. */
    cache_flush();
    cache_load(0);
    while (b < 31 && cache_load(1u << b))
        b++;

    /* TODO: Find s, E, policy and write_allocate, then report them here. */
    report(b, 0, 0, POLICY_LRU, 1);
}
