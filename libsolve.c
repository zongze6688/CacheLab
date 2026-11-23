/*
 * libsolve.c - Talks to test-solver for you. Do not modify.
 *
 * test-solver runs ./solver once per mystery cache. Every cache_* call is
 * a request over a pipe (fds 3 and 4); the cache itself lives in
 * test-solver. main() calls your solve() once, then exits.
 */
#include <stdlib.h>
#include <unistd.h>
#include "solver.h"

static int req_fd = 4, resp_fd = 3;

static int request(unsigned char op, uint32_t addr)
{
    unsigned char m[5] = { op, (unsigned char)addr, (unsigned char)(addr >> 8),
                           (unsigned char)(addr >> 16), (unsigned char)(addr >> 24) };
    unsigned char a;
    if (write(req_fd, m, 5) != 5 || read(resp_fd, &a, 1) != 1)
        _exit(3);                        /* test-solver stopped this run */
    return a;
}

int cache_load(uint32_t addr)
{
    return request('L', addr);
}

int cache_store(uint32_t addr)
{
    return request('S', addr);
}

void cache_flush(void)
{
    request('F', 0);
}

void report(int b, int s, int E, int policy, int write_allocate)
{
    unsigned char m[8] = { 'R', (unsigned char)b, (unsigned char)s, (unsigned char)E,
                           (unsigned char)policy, (unsigned char)write_allocate, 0, 0 };
    unsigned char a;
    if (write(req_fd, m, 8) != 8 || read(resp_fd, &a, 1) != 1)
        _exit(3);
}

int main(void)
{
    solve();
    request('Q', 0);
    return 0;
}
