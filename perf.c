/*
 * perf.c - Shared trace and miss-count logic for test-trans and test-mul.
 *
 * Based on eval_perf() in CMU's test-trans.c. Same trace filter: keep the
 * accesses from the start marker through the end marker whose address is
 * below 0xffffffff, which drops the stack. Two fixes: a run that crashes
 * or never reaches the end marker is invalid instead of being scored on a
 * partial trace, and a missing valgrind is reported as such.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "perf.h"

/*
 * extract_trace - Copy the marked region of trace.tmp into out_name.
 *     Returns 1 if the end marker was found, 0 otherwise.
 */
static int extract_trace(const char *out_name)
{
    unsigned long long marker_start, marker_end, addr;
    unsigned len;
    char buf[1000];
    int flag = 0, found_end = 0;
    FILE *marker_fp, *full_fp, *part_fp;

    marker_fp = fopen(".marker", "r");
    if (marker_fp == NULL)
        return 0;
    if (fscanf(marker_fp, "%llx %llx", &marker_start, &marker_end) != 2) {
        fclose(marker_fp);
        return 0;
    }
    fclose(marker_fp);

    full_fp = fopen("trace.tmp", "r");
    part_fp = fopen(out_name, "w");
    if (full_fp == NULL || part_fp == NULL) {
        if (full_fp)
            fclose(full_fp);
        if (part_fp)
            fclose(part_fp);
        return 0;
    }

    while (fgets(buf, sizeof(buf), full_fp) != NULL) {
        /* We are only interested in memory access instructions */
        if (!(buf[0] == ' ' && buf[2] == ' ' &&
              (buf[1] == 'S' || buf[1] == 'M' || buf[1] == 'L')))
            continue;
        if (sscanf(buf + 3, "%llx,%u", &addr, &len) != 2)
            continue;
        if (addr == marker_start)
            flag = 1;
        /* Ignore stack accesses: keep only the low 32-bit address space */
        if (flag && addr < 0xffffffff)
            fputs(buf, part_fp);
        if (flag && addr == marker_end) {
            found_end = 1;
            break;
        }
    }
    fclose(full_fp);
    fclose(part_fp);
    return found_end;
}

int eval_one(const char *tracegen, int M, int N, int fn,
             const char *trace_name, int s, int E, int b, unsigned r[3])
{
    char cmd[512];
    FILE *fp;
    int status, code;

    unlink(".marker");
    snprintf(cmd, sizeof(cmd),
             "valgrind --tool=lackey --trace-mem=yes --log-fd=1 -v "
             "./%s -M %d -N %d -F %d > trace.tmp 2>/dev/null",
             tracegen, M, N, fn);
    status = system(cmd);
    code = (status != -1 && WIFEXITED(status)) ? WEXITSTATUS(status) : -1;

    if (code == 127) {
        printf("Error: valgrind not found. Install valgrind and retry.\n");
        exit(1);
    }
    if (code != 0) {
        if (code > 0 && code <= 100)
            printf("Validation error at function %d! Run ./%s -M %d -N %d -F %d for details.\n",
                   code - 1, tracegen, M, N, fn);
        else
            printf("Function %d crashed (status %d). Run ./%s -M %d -N %d -F %d for details.\n",
                   fn, code, tracegen, M, N, fn);
        printf("Skipping performance evaluation for this function.\n");
        return 0;
    }
    if (!extract_trace(trace_name)) {
        printf("Error: end marker not found in the trace of function %d.\n", fn);
        return 0;
    }

    printf("Step 2: Evaluating performance (s=%d, E=%d, b=%d)\n", s, E, b);
    unlink(".csim_results");
    snprintf(cmd, sizeof(cmd), "./csim-ref -s %d -E %d -b %d -t %s > /dev/null",
             s, E, b, trace_name);
    if (system(cmd) != 0) {
        printf("Error: ./csim-ref failed.\n");
        return 0;
    }
    fp = fopen(".csim_results", "r");
    if (fp == NULL)
        return 0;
    code = fscanf(fp, "%u %u %u", &r[0], &r[1], &r[2]);
    fclose(fp);
    return code == 3;
}
