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
    for (int i = 0; i < n*n; i++) m[i] = val;
}

int main(int argc, char **argv) {

    // Optional: run like ./matrix_lab 6
    if (argc >= 2) {
        int t = atoi(argv[1]);
        if (t > 0) omp_set_num_threads(t);
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
    printf("\n----------1----------\n\n");
    double start = omp_get_wtime();

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = A[i * N + j] + B[i * N + j];
        }
    }
    
    double end = omp_get_wtime();
    printf("Level 1 Start Time: %f s\n", start);
    printf("Level 1 End Time: %f s\n", end);
    printf("Level 1 Time: %f s\n", end - start);
    printf("Level 1 Result C[N-1][N-1]: %f\n\n", C[(N-1)*N + (N-1)]);

    // =====================================================
    // LEVEL 2: MANUAL BLOCK DECOMPOSITION
    // =====================================================
    printf("\n----------2A----------\n\n");
    start = omp_get_wtime();
    #pragma omp parallel
{
    // TODO: Implement Level 2A here
    int id = omp_get_thread_num();
    int T = omp_get_num_threads();
    int chunk = N / T;
    int start_row = id * chunk;
    int end_row = start_row + chunk;

    for (int i = start_row; i < end_row; i++) {
        for (int j = start_row; j < end_row; j++) {
            C[i * N + j] = A[i * N + j] + B[i * N + j];
        }
    }

    end = omp_get_wtime();
    printf("The thread id is %d \n", id);
    printf("end_index=%d\n", end_row);
    printf("Level 1 Time: %f \n", end - start);
    printf("Level 1 Result C[N-1][N-1]: %f\n\n", C[(N-1)*N + (N-1)]);
}

    // TODO: Implement Level 2C here
    printf("\n----------2C----------\n\n");
    start = omp_get_wtime();
    #pragma omp parallel
{
    int id = omp_get_thread_num();
    int T = omp_get_num_threads();
    int chunk = N / T;
    int start_row = id * chunk;
    int end_row;
    if (id == T - 1) {
            end_row = N;
        } else {
            end_row = start_row + chunk;
        }

    for (int i = start_row; i < end_row; i++) {
        for (int j = start_row; j < end_row; j++) {
            C[i * N + j] = A[i * N + j] + B[i * N + j];
        }
    }

    end = omp_get_wtime();
    printf("The thread id is %d \n", id);
    printf("end_index=%d\n", end_row);
    printf("Level 1 Time: %f \n", end - start);
    printf("Level 1 Result C[N-1][N-1]: %f\n\n", C[(N-1)*N + (N-1)]);

    // =====================================================
    // LEVEL 3: CYCLIC DECOMPOSITION
    // =====================================================

    // TODO: Implement Level 3 here

    printf("\n----------3----------\n\n");
    start = omp_get_wtime();
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int T = omp_get_num_threads();
        for (int i = id; i < N; i += T) { // Round-robin rows
            for (int j = 0; j < N; j++) {
                C[i * N + j] = A[i * N + j] + B[i * N + j];
            }
        }
    }
    end = omp_get_wtime();
    printf("Level 3 Time: %f  \n", end - start);
    printf("Level 3 Result C[N-1][N-1]: %f\n\n", C[(N-1)*N + (N-1)]);

    
}
    // =====================================================
    // LEVEL 4: DEPENDENCY TRAP
    // =====================================================

    // TODO: Implement Level 4A here

    printf("\n----------4A----------\n\n");
    fill_matrix(C, N, 1.0); 
    for (int i = 1; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = C[i * N + j] + C[(i - 1) * N + j];
        }
    }
    printf("Level 4A Serial Checkpoint C[1000][0]: %f\n", C[1000 * N + 0]);


    // TODO: Implement Level 4B here

    printf("\n----------4B----------\n\n");
    fill_matrix(C, N, 1.0);
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int T = omp_get_num_threads();
        int chunk = N / T;
        int start_row = (id * chunk == 0) ? 1 : id * chunk; 
        int end_row = (id == T - 1) ? N : id * chunk + chunk;

        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < N; j++) {
                C[i * N + j] = C[i * N + j] + C[(i - 1) * N + j];
            }
        }
    }
    printf("Level 4B Parallel Checkpoint C[1000][0]: %f\n\n", C[1000 * N + 0]);


    // =====================================================
    // LEVEL 5: BARRIER SYNCHRONIZATION
    // =====================================================

    // TODO: Implement Level 5A here

    printf("\n----------5A----------\n\n");
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int T = omp_get_num_threads();
        int chunk = N / T;
        int start_row = id * chunk;
        int end_row = (id == T - 1) ? N : start_row + chunk;

        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < N; j++) {
                C[i * N + j] = A[i * N + j] + B[i * N + j];
            }
        }

        printf("Thread %d finished calculation\n", id);
        printf("Thread %d starting verification\n", id);
    }

    // TODO: Implement Level 5B here

    printf("\n----------5B----------\n\n");
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int T = omp_get_num_threads();
        int chunk = N / T;
        int start_row = id * chunk;
        int end_row = (id == T - 1) ? N : start_row + chunk;

        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < N; j++) {
                C[i * N + j] = A[i * N + j] + B[i * N + j];
            }
        }

        printf("Thread %d finished calculation\n", id);
        #pragma omp barrier 
        printf("Thread %d starting verification\n", id);
    }

    free(A);
    free(B);
    free(C);
    return 0;
}
