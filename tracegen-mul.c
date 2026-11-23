/*
 * tracegen-mul.c - Running the binary tracegen-mul with valgrind produces
 * a memory trace of all of the registered multiply functions.
 *
 * Same scheme as tracegen.c. The multiply must leave A and B unchanged
 * and write every element of C; C starts out holding garbage.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "cachelab.h"

/* External variables declared in cachelab.c */
extern mul_func_t mul_func_list[MAX_MUL_FUNCS];
extern int mul_func_counter;

/* External function from mul.c */
extern void registerFunctions();

/* Markers used to bound trace regions of interest */
volatile char MARKER_START, MARKER_END;

static struct {
    int A[256][256];
    int B[256][256];
    int C[256][256];
} mat __attribute__((aligned(4096)));
static int A_copy[256][256];
static int B_copy[256][256];
static int M;
static int N;

int validate(int fn, int M, int N, int A[N][M], int B[M][N], int C[N][N])
{
    int rC[N][N];
    if (memcmp(A_copy, mat.A, sizeof(A_copy)) != 0 ||
        memcmp(B_copy, mat.B, sizeof(B_copy)) != 0) {
        printf("Validation failed on function %d! Matrix A or B was modified\n", fn);
        return 0;
    }
    correctMul(M, N, A, B, rC);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (rC[i][j] != C[i][j]) {
                printf("Validation failed on function %d! Expected %d but got %d at C[%d][%d]\n",
                       fn, rC[i][j], C[i][j], i, j);
                return 0;
            }
        }
    }
    return 1;
}

/* run - run multiply function fn between the two markers and check it */
static int run(int fn)
{
    initMulMatrix(M, N, mat.A, mat.B, mat.C);
    memcpy(A_copy, mat.A, sizeof(A_copy));
    memcpy(B_copy, mat.B, sizeof(B_copy));
    MARKER_START = 33;
    (*mul_func_list[fn].func_ptr)(M, N, mat.A, mat.B, mat.C);
    MARKER_END = 34;
    return validate(fn, M, N, mat.A, mat.B, mat.C);
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
            printf("./tracegen-mul failed to parse its options.\n");
            exit(1);
        }
    }
    if (M <= 0 || N <= 0 || M > 256 || N > 256) {
        printf("./tracegen-mul: M and N must be in [1, 256]\n");
        exit(1);
    }

    /*  Register multiply functions */
    registerFunctions();
    if (selectedFunc >= mul_func_counter) {
        printf("./tracegen-mul: no function %d\n", selectedFunc);
        exit(1);
    }

    /* Record marker addresses */
    FILE *marker_fp = fopen(".marker", "w");
    assert(marker_fp);
    fprintf(marker_fp, "%llx %llx",
            (unsigned long long int)&MARKER_START,
            (unsigned long long int)&MARKER_END);
    fclose(marker_fp);

    if (-1 == selectedFunc) {
        for (i = 0; i < mul_func_counter; i++)
            if (!run(i))
                return i + 1;
    } else if (!run(selectedFunc)) {
        return selectedFunc + 1;
    }
    return 0;
}
