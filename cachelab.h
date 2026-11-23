/*
 * cachelab.h - Prototypes for Cache Lab helper functions
 */

#ifndef CACHELAB_TOOLS_H
#define CACHELAB_TOOLS_H

#define MAX_TRANS_FUNCS 100
#define MAX_MUL_FUNCS 100

typedef struct trans_func {
    void (*func_ptr)(int M, int N, int[N][M], int[M][N]);
    char *description;
    char correct;
    unsigned int num_hits;
    unsigned int num_misses;
    unsigned int num_evictions;
} trans_func_t;

typedef struct mul_func {
    void (*func_ptr)(int M, int N, int[N][M], int[M][N], int[N][N]);
    char *description;
    char correct;
    unsigned int num_hits;
    unsigned int num_misses;
    unsigned int num_evictions;
} mul_func_t;

/*
 * printSummary - This function provides a standard way for your cache
 * simulator to display its final hit and miss statistics
 */
void printSummary(int hits,       /* number of  hits */
                  int misses,     /* number of misses */
                  int evictions); /* number of evictions */

/* ---------- Part B: transpose ---------- */

/* Fill the matrix with data */
void initMatrix(int M, int N, int A[N][M], int B[M][N]);

/* The baseline trans function that produces correct results. */
void correctTrans(int M, int N, int A[N][M], int B[M][N]);

/* Add the given function to the function list */
void registerTransFunction(
    void (*trans)(int M, int N, int[N][M], int[M][N]), char *desc);

/* ---------- Part C: multiply ---------- */

/* Fill A (N x M) and B (M x N) with small random values, C with garbage */
void initMulMatrix(int M, int N, int A[N][M], int B[M][N], int C[N][N]);

/* The baseline multiply function that produces correct results. */
void correctMul(int M, int N, int A[N][M], int B[M][N], int C[N][N]);

/* Add the given function to the function list */
void registerMulFunction(
    void (*mul)(int M, int N, int[N][M], int[M][N], int[N][N]), char *desc);

#endif /* CACHELAB_TOOLS_H */
