#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 4000

double* alloc_matrix(int n) {
    double *m = (double*) malloc((size_t)n * (size_t)n * sizeof(double));
    if (!m) {
        printf("Allocation failed!\n");
        exit(1);
    }
    return m;
}

void fill_matrix(double *m, int n, double val) {
    for (int i = 0; i < n * n; i++) {
        m[i] = val;
    }
}

int main(int argc, char **argv) {

    // Optional: run like ./matrix_lab 6
    if (argc >= 2) {
        int t = atoi(argv[1]);
        if (t > 0) {
            omp_set_num_threads(t);
        }
    }

    double *A = alloc_matrix(N);
    double *B = alloc_matrix(N);
    double *C = alloc_matrix(N);

    fill_matrix(A, N, 1.0);
    fill_matrix(B, N, 1.0);
    fill_matrix(C, N, 0.0);

    // =====================================================
    // LEVEL 1: SERIAL BASELINE
    // =====================================================

    // TODO: Implement Level 1 here
    fill_matrix(C, N, 0.0);

    double t0 = omp_get_wtime();

    // Serial matrix addition: C = A + B
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = A[i * N + j] + B[i * N + j];
        }
    }

    double t1 = omp_get_wtime();

    printf("Level 1 Serial Time = %.6f seconds\n", t1 - t0);
    printf("C[N-1][N-1] = %.1f\n", C[(N - 1) * N + (N - 1)]);

    // =====================================================
    // LEVEL 2: MANUAL BLOCK DECOMPOSITION
    // =====================================================

    // =====================================================
    // LEVEL 2 - Task A: Manual Block Decomposition (NAIVE)
    // No remainder fix yet
    // =====================================================

    fill_matrix(C, N, 0.0);

    t0 = omp_get_wtime();

#pragma omp parallel
    {
        int id = omp_get_thread_num();
        int T  = omp_get_num_threads();

        int chunk = N / T;
        int start_row = id * chunk;
        int end_row   = start_row + chunk;

        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < N; j++) {
                C[i * N + j] = A[i * N + j] + B[i * N + j];
            }
        }
    }

    t1 = omp_get_wtime();

    printf("Level 2A Block NAIVE Time = %.6f seconds\n", t1 - t0);
    printf("C[N-1][N-1] = %.1f\n", C[(N - 1) * N + (N - 1)]);

    // =====================================================
    // LEVEL 2 - Task C
    // =====================================================

    // =====================================================
    // LEVEL 2 - Task C: Manual Block Decomposition (FIXED)
    // Fix remainder when N is not divisible by T
    // =====================================================

    fill_matrix(C, N, 0.0);

    t0 = omp_get_wtime();

#pragma omp parallel
    {
        int id = omp_get_thread_num();
        int T  = omp_get_num_threads();

        int chunk = N / T;
        int start_row = id * chunk;
        int end_row   = start_row + chunk;

        // Fix: last thread takes any leftover rows
        if (id == T - 1) {
            end_row = N;
        }

        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < N; j++) {
                C[i * N + j] = A[i * N + j] + B[i * N + j];
            }
        }
    }

    t1 = omp_get_wtime();

    printf("Level 2C Block FIXED Time = %.6f seconds\n", t1 - t0);
    printf("C[N-1][N-1] = %.1f\n", C[(N - 1) * N + (N - 1)]);

    // =====================================================
    // LEVEL 3: CYCLIC DECOMPOSITION
    // =====================================================

    // =====================================================
    // LEVEL 3: CYCLIC DECOMPOSITION (SPMD) — KEY SOLUTION
    // Constraint: NO omp for / parallel for
    // Round-robin rows: thread id, id+T, id+2T, ...
    // =====================================================

    fill_matrix(C, N, 0.0);

    t0 = omp_get_wtime();

#pragma omp parallel
    {
        int id = omp_get_thread_num();
        int T  = omp_get_num_threads();

        for (int i = id; i < N; i += T) {
            for (int j = 0; j < N; j++) {
                C[i * N + j] = A[i * N + j] + B[i * N + j];
            }
        }
    }

    t1 = omp_get_wtime();

    printf("Level 3 Cyclic (manual SPMD) Time = %.6f seconds\n", t1 - t0);
    printf("C[N-1][N-1] = %.1f\n", C[(N - 1) * N + (N - 1)]);

    // =====================================================
    // LEVEL 4: DEPENDENCY TRAP TASK A
    // =====================================================

    // =====================================================
    // LEVEL 4 - Task A: SERIAL Row Accumulation (CORRECT)
    // Kernel: C[i][j] = C[i][j] + C[i-1][j]
    // Do this in serial first to get the correct reference output.
    // =====================================================

    fill_matrix(C, N, 1.0);   // known starting state

    t0 = omp_get_wtime();

    // Start from i = 1 because we use i-1
    for (int i = 1; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = C[i * N + j] + C[(i - 1) * N + j];
        }
    }

    t1 = omp_get_wtime();

    printf("Level 4A Serial Accumulation Time = %.6f seconds\n", t1 - t0);
    printf("Checkpoint C[1000][0] = %.1f\n", C[1000 * N + 0]);

    // =====================================================
    // LEVEL 4 - Task B: NAIVE PARALLEL ATTEMPT (EXPECTED WRONG)
    // Manual Block Decomposition applied to a loop with dependency.
    // Kernel: C[i][j] = C[i][j] + C[i-1][j]
    // =====================================================

    // Reset to the same known starting state as Task A
    fill_matrix(C, N, 1.0);

    t0 = omp_get_wtime();

#pragma omp parallel
    {
        int id = omp_get_thread_num();
        int T  = omp_get_num_threads();

        int chunk = N / T;
        int start_row = id * chunk;
        int end_row   = start_row + chunk;

        // remainder fix (still does NOT solve dependency)
        if (id == T - 1) {
            end_row = N;
        }

        // Avoid i-1 underflow for the first block
        if (start_row < 1) {
            start_row = 1;
        }

        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < N; j++) {
                C[i * N + j] = C[i * N + j] + C[(i - 1) * N + j];
            }
        }
    }

    t1 = omp_get_wtime();

    printf("Level 4B Naive Parallel Accumulation Time = %.6f seconds\n", t1 - t0);
    printf("Checkpoint C[1000][0] = %.1f\n", C[1000 * N + 0]);

    // =====================================================
    // LEVEL 5: BARRIER SYNCHRONIZATION
    // =====================================================

    // =====================================================
    // LEVEL 5 - Task A: Demonstrate lack of synchronization
    // Add prints after the Level-2 manual block calculation.
    // Do NOT add a barrier yet (that is Task B).
    // =====================================================

    fill_matrix(C, N, 0.0);

    t0 = omp_get_wtime();

#pragma omp parallel
    {
        int id = omp_get_thread_num();
        int T  = omp_get_num_threads();

        int chunk = N / T;
        int start_row = id * chunk;
        int end_row   = start_row + chunk;

        // remainder fix (keep it, so correctness isn't the issue here)
        if (id == T - 1) {
            end_row = N;
        }

        // Calculation phase
        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < N; j++) {
                C[i * N + j] = A[i * N + j] + B[i * N + j];
            }
        }

        // Prints with NO barrier in between (this is the point of Task A)
        printf("Thread %d finished calculation\n", id);
        printf("Thread %d starting verification\n", id);

        // Optional: simple verification print by one thread (not required)
        // if (id == 0) printf("C[N-1][N-1] = %.1f\n", C[(N-1)*N + (N-1)]);
    }

    t1 = omp_get_wtime();

    printf("Level 5A Time (no barrier) = %.6f seconds\n", t1 - t0);

    // =====================================================
    // LEVEL 5 - Task B: Fix with a barrier
    // Insert #pragma omp barrier between calculation and verification prints.
    // =====================================================

    fill_matrix(C, N, 0.0);

    t0 = omp_get_wtime();

#pragma omp parallel
    {
        int id = omp_get_thread_num();
        int T  = omp_get_num_threads();

        int chunk = N / T;
        int start_row = id * chunk;
        int end_row   = start_row + chunk;

        // remainder fix
        if (id == T - 1) {
            end_row = N;
        }

        // Calculation phase
        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < N; j++) {
                C[i * N + j] = A[i * N + j] + B[i * N + j];
            }
        }

        printf("Thread %d finished calculation\n", id);

        // Barrier: everyone must reach here before anyone continues
#pragma omp barrier

        printf("Thread %d starting verification\n", id);

        // Optional: one-thread verification print
        // if (id == 0) printf("C[N-1][N-1] = %.1f\n", C[(N-1)*N + (N-1)]);
    }

    t1 = omp_get_wtime();

    printf("Level 5B Time (with barrier) = %.6f seconds\n", t1 - t0);

    free(A);
    free(B);
    free(C);
    return 0;
}
