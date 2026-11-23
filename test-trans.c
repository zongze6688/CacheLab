/*
 * test-trans.c - Checks the correctness and performance of all of the
 *     student's transpose functions and records the results for their
 *     official submitted version as well.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include "cachelab.h"
#include "perf.h"

/* Maximum array dimension */
#define MAXN 256

/* The description string for the transpose_submit() function that the
   student submits for credit */
#define SUBMIT_DESCRIPTION "Transpose submission"

/* External function defined in trans.c */
extern void registerFunctions();

/* External variables defined in cachelab.c */
extern trans_func_t func_list[MAX_TRANS_FUNCS];
extern int func_counter;

static void usage(char *argv[])
{
    printf("Usage: %s [-h] -M <cols> -N <rows>\n", argv[0]);
    printf("Options:\n");
    printf("  -h          Print this help message.\n");
    printf("  -M <cols>   Number of columns of A (max %d)\n", MAXN);
    printf("  -N <rows>   Number of rows of A (max %d)\n", MAXN);
    printf("Example: %s -M 8 -N 8\n", argv[0]);
}

static void sigsegv_handler(int signum)
{
    (void)signum;
    printf("Error: Segmentation Fault.\n");
    printf("TEST_TRANS_RESULTS=0:0\n");
    fflush(stdout);
    exit(1);
}

static void sigalrm_handler(int signum)
{
    (void)signum;
    printf("Error: Program timed out.\n");
    printf("TEST_TRANS_RESULTS=0:0\n");
    fflush(stdout);
    exit(1);
}

int main(int argc, char *argv[])
{
    int M = 0, N = 0, c, i;
    int submit = -1, submit_correct = 0, submit_misses = INT_MAX;
    unsigned r[3];
    char trace_name[64];

    while ((c = getopt(argc, argv, "M:N:h")) != -1) {
        switch (c) {
        case 'M':
            M = atoi(optarg);
            break;
        case 'N':
            N = atoi(optarg);
            break;
        case 'h':
            usage(argv);
            exit(0);
        default:
            usage(argv);
            exit(1);
        }
    }
    if (M <= 0 || N <= 0 || M > MAXN || N > MAXN) {
        printf("Error: M and N must be in [1, %d]\n", MAXN);
        usage(argv);
        exit(1);
    }

    signal(SIGSEGV, sigsegv_handler);
    signal(SIGALRM, sigalrm_handler);
    alarm(360);

    registerFunctions();
    for (i = 0; i < func_counter; i++) {
        if (strcmp(func_list[i].description, SUBMIT_DESCRIPTION) == 0)
            submit = i;
        printf("\nFunction %d (%d total)\n", i, func_counter);
        printf("Step 1: Validating and generating memory traces\n");
        snprintf(trace_name, sizeof(trace_name), "trace.f%d", i);
        if (!eval_one("tracegen", M, N, i, trace_name, 5, 1, 5, r))
            continue;
        func_list[i].correct = 1;
        func_list[i].num_hits = r[0];
        func_list[i].num_misses = r[1];
        func_list[i].num_evictions = r[2];
        printf("func %d (%s): hits:%u, misses:%u, evictions:%u\n",
               i, func_list[i].description, r[0], r[1], r[2]);
        if (submit == i) {
            submit_correct = 1;
            submit_misses = (int)r[1];
        }
    }

    if (submit == -1) {
        printf("\nError: We could not find your transpose_submit() function\n");
        printf("Error: Please ensure that description field is exactly \"%s\"\n",
               SUBMIT_DESCRIPTION);
        printf("\nTEST_TRANS_RESULTS=0:0\n");
    } else {
        printf("\nSummary for official submission (func %d): correctness=%d misses=%d\n",
               submit, submit_correct, submit_misses);
        printf("\nTEST_TRANS_RESULTS=%d:%d\n", submit_correct, submit_misses);
    }
    return 0;
}
