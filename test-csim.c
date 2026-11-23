/*
 * test-csim.c - Compare the student's cache simulator (./csim) with the
 *     reference simulator (./csim-ref) on eight fixed test cases.
 *
 * The first seven tests are worth 3 points each and the last one 6
 * points. The points of a test are split evenly across hits, misses and
 * evictions. Extra verbose and boundary checks are diagnostic only.
 * The last line is TEST_CSIM_RESULTS=<points> for driver.py.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int s, E, b;
    const char *trace;
    int points;
} test_t;

static const test_t tests[] = {
    {1, 1, 1, "traces/yi2.trace", 3},
    {4, 2, 4, "traces/yi.trace", 3},
    {2, 1, 4, "traces/dave.trace", 3},
    {2, 1, 3, "traces/trans.trace", 3},
    {2, 2, 3, "traces/trans.trace", 3},
    {2, 4, 3, "traces/trans.trace", 3},
    {5, 1, 5, "traces/trans.trace", 3},
    {5, 1, 5, "traces/long.trace", 6},
};

static const test_t diagnostic_tests[] = {
    {4, 1, 4, "traces/yi.trace", 0},
    {0, 2, 0, "traces/csim-boundary.trace", 0},
    {0, 1, 64, "traces/csim-boundary.trace", 0},
    {1, 1, 63, "traces/csim-boundary.trace", 0},
};

/*
 * run - Run one simulator on test t. Returns 0 and fills r[] with
 *     hits, misses and evictions, or returns -1 if the run or result is invalid.
 */
static int run(const char *prog, const test_t *t, int r[3])
{
    char cmd[512];
    FILE *fp;
    int n;

    unlink(".csim_results");
    snprintf(cmd, sizeof(cmd), "./%s -s %d -E %d -b %d -t %s > /dev/null 2>&1",
             prog, t->s, t->E, t->b, t->trace);
    if (system(cmd) != 0)
        return -1;
    fp = fopen(".csim_results", "r");
    if (fp == NULL)
        return -1;
    n = fscanf(fp, "%d %d %d", &r[0], &r[1], &r[2]);
    fclose(fp);
    return n == 3 ? 0 : -1;
}

/* Capture the complete stdout of a simulator on one short diagnostic trace. */
static int read_verbose(const char *prog, const test_t *t, char *output, size_t capacity)
{
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "./%s -v -s %d -E %d -b %d -t %s 2>/dev/null",
             prog, t->s, t->E, t->b, t->trace);

    unlink(".csim_results");
    FILE *fp = popen(cmd, "r");
    if (fp == NULL)
        return -1;
    size_t length = fread(output, 1, capacity, fp);
    int complete = feof(fp) && !ferror(fp);
    if (pclose(fp) != 0 || !complete)
        return -1;
    return (int)length;
}

static void check_diagnostics(void)
{
    printf("\nNon-scoring checks (-v output)\n");
    for (size_t i = 0; i < sizeof(diagnostic_tests) / sizeof(diagnostic_tests[0]); i++) {
        const test_t *t = &diagnostic_tests[i];
        char expected[1024], actual[1024];
        int expected_length = read_verbose("csim-ref", t, expected, sizeof(expected));
        const char *status = "ERROR (csim-ref)";
        if (expected_length >= 0) {
            int actual_length = read_verbose("csim", t, actual, sizeof(actual));
            int matches = actual_length == expected_length &&
                memcmp(actual, expected, (size_t)expected_length) == 0;
            status = matches ? "PASS" : "FAIL";
        }
        printf("  (%d,%d,%d) %s  %s\n", t->s, t->E, t->b, t->trace, status);
    }
}

int main(void)
{
    int ntests = sizeof(tests) / sizeof(tests[0]);
    int total = 0;
    int i, k;

    printf("                        Your simulator     Reference simulator\n");
    printf("Points (s,E,b)    Hits  Misses  Evicts    Hits  Misses  Evicts\n");

    for (i = 0; i < ntests; i++) {
        const test_t *t = &tests[i];
        int ref[3], stu[3] = {-1, -1, -1};
        int points = 0;

        if (run("csim-ref", t, ref) != 0) {
            printf("Error: ./csim-ref failed on %s\n", t->trace);
            return 1;
        }
        if (run("csim", t, stu) == 0) {
            for (k = 0; k < 3; k++)
                if (stu[k] == ref[k])
                    points += t->points / 3;
        }
        total += points;

        printf("%6d (%d,%d,%d)%8d%8d%8d%8d%8d%8d  %s\n",
               points, t->s, t->E, t->b,
               stu[0], stu[1], stu[2], ref[0], ref[1], ref[2], t->trace);
    }
    printf("%6d\n", total);
    check_diagnostics();
    printf("\nTEST_CSIM_RESULTS=%d\n", total);
    return 0;
}
