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

    double start;
    double end;
    start = omp_get_wtime();
    for(int i = 0; i<N; i++){
        for(int j = 0; j<N; j++){
            C[i*N+j] = A[i*N+j]+B[i*N+j];
        }
    }
    printf("C[N-1][N-1] is equal to: %f\n", C[N*N-1]);
    end = omp_get_wtime();
    printf("Level 1 took %f seconds\n", end-start);

    // =====================================================
    // LEVEL 2: MANUAL BLOCK DECOMPOSITION
    // =====================================================

    // TODO: Implement Level 2A here

    fill_matrix(C, N, 0.0);

    start = omp_get_wtime();
    #pragma omp parallel
    {
        int total = omp_get_num_threads();
        int id = omp_get_thread_num();
        double chunk = N/total;
        int start_row = id*chunk;
        int end_row = start_row+chunk;
        for(int i = start_row; i<end_row; i++){
            for(int j = 0; j<N; j++){
                C[i*N+j] = A[i*N+j]+B[i*N+j];
            }
        }
    }
    printf("C[N-1][N-1] is equal to: %f\n", C[N*N-1]);
    end = omp_get_wtime();
    printf("Level 2A took %f seconds\n", end-start);

    // TODO: Implement Level 2C here

    fill_matrix(C, N, 0.0);

    start = omp_get_wtime();
    #pragma omp parallel
    {
        int total = omp_get_num_threads();
        int id = omp_get_thread_num();
        double chunk = N/total;
        int remainder = N%total;
        int start_row = id*chunk;
        int end_row = start_row+chunk;
        if(id==total-1){end_row = end_row+remainder;}
        for(int i = start_row; i<end_row; i++){
            for(int j = 0; j<N; j++){
                C[i*N+j] = A[i*N+j]+B[i*N+j];
            }
        }
    }
    printf("C[N-1][N-1] is equal to: %f\n", C[N*N-1]);
    end = omp_get_wtime();
    printf("Level 2C took %f seconds\n", end-start);

    // =====================================================
    // LEVEL 3: CYCLIC DECOMPOSITION
    // =====================================================

    // TODO: Implement Level 3 here

    fill_matrix(C, N, 0.0);

    start = omp_get_wtime();
    #pragma omp parallel
    {
        int total = omp_get_num_threads();
        int id = omp_get_thread_num();
        for(int i = id; i<N; i+=total){
            for(int j = 0; j<N; j++){
                C[i*N+j] = A[i*N+j]+B[i*N+j];
            }
        }
    }
    printf("C[N-1][N-1] is equal to: %f\n", C[N*N-1]);
    end = omp_get_wtime();
    printf("Level 3 took %f seconds\n", end-start);

    // =====================================================
    // LEVEL 4: DEPENDENCY TRAP
    // =====================================================

    // TODO: Implement Level 4A here

    fill_matrix(C, N, 1.0);

    for(int i = 0; i<N; i++){
        for(int j = 0; j<N; j++){
            C[i*N+j] = C[i*N+j]+C[(i-1)*N+j];
        }
    }
    printf("C[1000][0] (serial) is equal to: %f\n", C[N*1000]);

    // TODO: Implement Level 4B here

    fill_matrix(C, N, 1.0);

    #pragma omp parallel
    {
        int total = omp_get_num_threads();
        int id = omp_get_thread_num();
        double chunk = N/total;
        int start_row = id*chunk;
        int end_row = start_row+chunk;
        for(int i = start_row; i<end_row; i++){
            for(int j = 0; j<N; j++){
                C[i*N+j] = C[i*N+j]+C[(i-1)*N+j];
            }
        }
    }
    printf("C[1000][0] (parallel) is equal to: %f\n", C[N*1000]);

    // =====================================================
    // LEVEL 5: BARRIER SYNCHRONIZATION
    // =====================================================

    // TODO: Implement Level 5A here

    fill_matrix(C, N, 0.0);

    printf("Level 5A:\n");
    #pragma omp parallel
    {
        int total = omp_get_num_threads();
        int id = omp_get_thread_num();
        double chunk = N/total;
        int start_row = id*chunk;
        int end_row = start_row+chunk;
        for(int i = start_row; i<end_row; i++){
            for(int j = 0; j<N; j++){
                C[i*N+j] = A[i*N+j]+B[i*N+j];
            }
        }
        printf("Thread %d finished calculation\n", id);
        printf("Thread %d started verification\n", id);
    }

    // TODO: Implement Level 5B here

    fill_matrix(C, N, 0.0);

    printf("Level 5B:\n");
    #pragma omp parallel
    {
        int total = omp_get_num_threads();
        int id = omp_get_thread_num();
        double chunk = N/total;
        int start_row = id*chunk;
        int end_row = start_row+chunk;
        for(int i = start_row; i<end_row; i++){
            for(int j = 0; j<N; j++){
                C[i*N+j] = A[i*N+j]+B[i*N+j];
            }
        }
        printf("Thread %d finished calculation\n", id);
        #pragma omp barrier
        printf("Thread %d started verification\n", id);
    }

    free(A);
    free(B);
    free(C);
    return 0;
}
