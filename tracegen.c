/*
 * tracegen.c - Running the binary tracegen with valgrind produces
 * a memory trace of all of the registered transpose functions.
 *
 * The beginning and end of each registered transpose function's trace
 * is indicated by writing to "marker" addresses. These two marker
 * addresses are recorded in file for later use.
 *
 * Changes from the CMU version:
 *   - A and B live in one 4096-byte aligned struct, so A starts on a
 *     cache block and B is exactly 256KB after A on every platform
 *     (plain static arrays are only 8-byte aligned by gcc on aarch64).
 *   - The transpose must leave A unchanged.
 *   - getopt's result is stored in an int (char is unsigned on aarch64).
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "cachelab.h"

/* External variables declared in cachelab.c */
extern trans_func_t func_list[MAX_TRANS_FUNCS];
extern int func_counter;

/* External function from trans.c */
extern void registerFunctions();

/* Markers used to bound trace regions of interest */
volatile char MARKER_START, MARKER_END;

static struct {
    int A[256][256];
    int B[256][256];
} mat __attribute__((aligned(4096)));
static int A_copy[256][256];
static int M;
static int N;

int validate(int fn, int M, int N, int A[N][M], int B[M][N])
{
    int C[M][N];
    if (memcmp(A_copy, mat.A, sizeof(A_copy)) != 0) {
        printf("Validation failed on function %d! Matrix A was modified\n", fn);
        return 0;
    }
    memset(C, 0, sizeof(C));
    correctTrans(M, N, A, C);
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            if (B[i][j] != C[i][j]) {
                printf("Validation failed on function %d! Expected %d but got %d at B[%d][%d]\n",
                       fn, C[i][j], B[i][j], i, j);
                return 0;
            }
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    int i;
    int c;
    int selectedFunc = -1;

    while ((c = getopt(argc, argv, "M:N:F:")) != -1) {
        switch (c) {
        case 'M':
            M = atoi(optarg);
            break;
        case 'N':
            N = atoi(optarg);
            break;
        case 'F':
            selectedFunc = atoi(optarg);
            break;
        case '?':
        default:
            printf("./tracegen failed to parse its options.\n");
            exit(1);
        }
    }
    if (M <= 0 || N <= 0 || M > 256 || N > 256) {
        printf("./tracegen: M and N must be in [1, 256]\n");
        exit(1);
    }

    /*  Register transpose functions */
    registerFunctions();
    if (selectedFunc >= func_counter) {
        printf("./tracegen: no function %d\n", selectedFunc);
        exit(1);
    }

    /* Fill A with data */
    initMatrix(M, N, mat.A, mat.B);
    memcpy(A_copy, mat.A, sizeof(A_copy));

    /* Record marker addresses */
    FILE *marker_fp = fopen(".marker", "w");
    assert(marker_fp);
    fprintf(marker_fp, "%llx %llx",
            (unsigned long long int)&MARKER_START,
            (unsigned long long int)&MARKER_END);
    fclose(marker_fp);

    if (-1 == selectedFunc) {
        /* Invoke registered transpose functions */
        for (i = 0; i < func_counter; i++) {
            MARKER_START = 33;
            (*func_list[i].func_ptr)(M, N, mat.A, mat.B);
            MARKER_END = 34;
            if (!validate(i, M, N, mat.A, mat.B))
                return i + 1;
        }
    } else {
        MARKER_START = 33;
        (*func_list[selectedFunc].func_ptr)(M, N, mat.A, mat.B);
        MARKER_END = 34;
        if (!validate(selectedFunc, M, N, mat.A, mat.B))
            return selectedFunc + 1;
    }
    return 0;
}
