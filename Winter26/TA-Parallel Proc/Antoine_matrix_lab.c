#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 4000

#define real_index(i, j) i*N + j

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

    printf("thread num=%d\n", omp_get_max_threads());

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

    double start = omp_get_wtime();

    for (int i=0; i<N; i++){
        for (int j=0; j<N; j++){
            C[real_index(i, j)] = A[real_index(i, j)] + B[real_index(i, j)];
        }
    }

    double end = omp_get_wtime();
    printf("C[N-1][N-1]=%lf, final time=%lf\n", C[real_index(N-1, N-1)], end-start);

    // =====================================================
    // LEVEL 2: MANUAL BLOCK DECOMPOSITION
    // =====================================================

    // TODO: Implement Level 2A here

    // refill the matrixes
    fill_matrix(A, N, 1.0);
    fill_matrix(B, N, 1.0);
    fill_matrix(C, N, 0.0);

    start = omp_get_wtime();

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int thrn = omp_get_num_threads();

        int chunk = N/thrn;

        int start_row = chunk*id;
        int end_row = start_row + chunk;
        for (int i=start_row; i<end_row; i++){
            for (int j=0; j<N; j++){
                C[real_index(i, j)] = A[real_index(i, j)] + B[real_index(i, j)];
            }
        }
    }

    end = omp_get_wtime();
    printf("C[N-1][N-1]=%lf, final time=%lf\n", C[N*N-1], end-start);

    // TODO: Implement Level 2C here

    // refill the matrixes
    fill_matrix(A, N, 1.0);
    fill_matrix(B, N, 1.0);
    fill_matrix(C, N, 0.0);

    start = omp_get_wtime();

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int thrn = omp_get_num_threads();

        int chunk = N/thrn;

        int start_row = chunk*id;
        int end_row = start_row + chunk + ((id == thrn-1 ) ? N%thrn : 0); // if this is the last thread and there's a leftover give it to this one
        for (int i=start_row; i<end_row; i++){
            for (int j=0; j<N; j++){
                C[real_index(i, j)] = A[real_index(i, j)] + B[real_index(i, j)];
            }
        }
    }

    end = omp_get_wtime();
    printf("C[N-1][N-1]=%lf, final time=%lf\n", C[N*N-1], end-start);

    // =====================================================
    // LEVEL 3: CYCLIC DECOMPOSITION
    // =====================================================

    // TODO: Implement Level 3 here
    // refill the matrixes
    fill_matrix(A, N, 1.0);
    fill_matrix(B, N, 1.0);
    fill_matrix(C, N, 0.0);

    start = omp_get_wtime();

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int thrn = omp_get_num_threads();

        for (int i=id; i<N; i+=thrn){
            for (int j=id; j<N; j+=thrn){
                C[real_index(i, j)] = A[real_index(i, j)] + B[real_index(i, j)];
            }
        }
    }

    end = omp_get_wtime();
    printf("C[N-1][N-1]=%lf, final time=%lf\n", C[N*N-1], end-start);

    // =====================================================
    // LEVEL 4: DEPENDENCY TRAP
    // =====================================================

    // TODO: Implement Level 4A here
    // refill the matrixes
    fill_matrix(C, N, 1.0);

    start = omp_get_wtime();

    for (int i=1; i<N; i++){
        for (int j=0;j<N; j++){
            C[real_index(i, j)] = C[real_index(i, j)] + C[real_index(i-1, j)];
        }
        if (i == 1000)
            printf("C[1000][0] = %lf\n", C[real_index(1000, 0)]);
    }

    end = omp_get_wtime();
    printf("C[N-1][N-1]=%lf, final time=%lf\n", C[N*N-1], end-start);

    // TODO: Implement Level 4B here

    // refill the matrixes
    fill_matrix(C, N, 0.0);

    start = omp_get_wtime();

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int thrn = omp_get_num_threads();

        int chunk = N/thrn;

        int start_row = chunk*id;
        int end_row = start_row + chunk + ((id == thrn-1 ) ? N%thrn : 0); // if this is the last thread and there's a leftover give it to this one

        for (int i=start_row; i<end_row; i++){
            for (int j=0; j<N; j++){
                C[real_index(i, j)] = C[real_index(i, j)] + C[real_index(i-1, j)];
            }
            if (i == 1000){
                printf("C[1000][0] = %lf\n", C[real_index(1000, 0)]);
            }
        }
    }

    end = omp_get_wtime();
    printf("C[N-1][N-1]=%lf, final time=%lf\n", C[N*N-1], end-start);
    // =====================================================
    // LEVEL 5: BARRIER SYNCHRONIZATION
    // =====================================================

    // TODO: Implement Level 5A here
    // refill the matrixes
    fill_matrix(C, N, 0.0);

    start = omp_get_wtime();

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int thrn = omp_get_num_threads();

        int chunk = N/thrn;

        int start_row = chunk*id;
        int end_row = start_row + chunk + ((id == thrn-1 ) ? N%thrn : 0); // if this is the last thread and there's a leftover give it to this one

        for (int i=start_row; i<end_row; i++){
            for (int j=0; j<N; j++){
                C[real_index(i, j)] = C[real_index(i, j)] + C[real_index(i-1, j)];
            }
            if (i == 1000){
                printf("C[1000][0] = %lf\n", C[real_index(1000, 0)]);
            }
        }
        printf("Thread %d finished calculation\n", id);
        printf("Thread %d starting verification\n", id);
        for (int i=start_row; i<end_row; i++){
            for (int j=0; j<N; j++){
                if (C[real_index(i, j)] != 2.0){
                    // do nothing
                }
            }
        }
    }

    end = omp_get_wtime();
    printf("C[N-1][N-1]=%lf, final time=%lf\n", C[N*N-1], end-start);

    // TODO: Implement Level 5B here
    //
    // refill the matrixes
    fill_matrix(C, N, 0.0);

    start = omp_get_wtime();

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int thrn = omp_get_num_threads();

        int chunk = N/thrn;

        int start_row = chunk*id;
        int end_row = start_row + chunk + ((id == thrn-1 ) ? N%thrn : 0); // if this is the last thread and there's a leftover give it to this one

        for (int i=start_row; i<end_row; i++){
            for (int j=0; j<N; j++){
                C[real_index(i, j)] = C[real_index(i, j)] + C[real_index(i-1, j)];
            }
            if (i == 1000){
                printf("C[1000][0] = %lf\n", C[real_index(1000, 0)]);
            }
        }
        printf("Thread %d finished calculation\n", id);

        #pragma omp barrier
        printf("Thread %d starting verification\n", id);
        for (int i=start_row; i<end_row; i++){
            for (int j=0; j<N; j++){
                if (C[real_index(i, j)] != 2.0){
                    // do nothing
                }
            }
        }
    }

    end = omp_get_wtime();
    printf("C[N-1][N-1]=%lf, final time=%lf\n", C[N*N-1], end-start);

    free(A);
    free(B);
    free(C);
    return 0;
}
