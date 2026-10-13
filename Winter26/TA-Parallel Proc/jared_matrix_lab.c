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
    printf("\nLevel 1:\n");
    double start, end;
    start = omp_get_wtime();
    for(int i = 0; i<N; i++){
        for(int j = 0; j < N; j++){
            C[(i*N) + j] = (A[i * N + j] + B[i * N + j]);
        }
        
    }
    end = omp_get_wtime() - start;
    printf("Elapsed time serial = %.10f\n", end);
    printf("Last element = %.2f\n", C[(N*N)-1]);
    // =====================================================
    // LEVEL 2: MANUAL BLOCK DECOMPOSITION
    // =====================================================

    // TODO: Implement Level 2A here
    printf("\nLevel 2 A:\n");
    fill_matrix(A, N, 1.0);
    fill_matrix(B, N, 1.0);
    fill_matrix(C, N, 0.0);

    start = omp_get_wtime();
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int nthr = omp_get_num_threads();

        int chunk = N/nthr;
        int start = id * chunk;
        int end = start + chunk;

        for(int i = start; i<end; i++){
            for(int j = i; j < end; j++){
                C[(i*end) + j] = (A[i * end+ j] + B[i * end + j]);
            }
        }
        
    }
    end = omp_get_wtime() - start;
    printf("Elapsed time manual block scheduling bug error = %.10f\n", end);
    printf("Last element = %.2f\n", C[(N*N)-1]);


    // if you run it wihtout a right number of threads some of the values dont get added butif we add the fix above it works fine 

    // TODO: Implement Level 2C here
    printf("\nLevel 2 C:\n");
    fill_matrix(A, N, 1.0);
    fill_matrix(B, N, 1.0);
    fill_matrix(C, N, 0.0);

    start = omp_get_wtime();
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int nthr = omp_get_num_threads();

        int chunk = N/nthr;
        int start = id * chunk;
        int end = start + chunk;

        if(id == nthr -1){
            end = N;
        }

        for(int i = start; i<end; i++){
            for(int j = i; j < end; j++){
                C[(i*end) + j] = (A[i * end+ j] + B[i * end + j]);
            }
        }
        
    }
    end = omp_get_wtime() - start;
    printf("Elapsed time manual block scheduling with bug fix = %.10f\n", end);
    printf("Last element = %.2f\n", C[(N*N)-1]);

    // =====================================================
    // LEVEL 3: CYCLIC DECOMPOSITION
    // =====================================================\

    fill_matrix(A, N, 1.0);
    fill_matrix(B, N, 1.0);
    fill_matrix(C, N, 0.0);
    printf("\nLevel 3:\n");
    start = omp_get_wtime();
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int nthr = omp_get_num_threads();

        for(int i = id; i< N*N; i += nthr){
            C[i] = A[i] + B[i];
        }
        
    }
    end = omp_get_wtime() - start;
    printf("Last element = %.2f\n", C[(N*N)-1]);
    printf("Elapsed time Cyclic = %.10f\n", end);

    // it does manually fix the error because one of the threads will get to N*N eventually 

    // =====================================================
    // LEVEL 4: DEPENDENCY TRAP
    // =====================================================

    // TODO: Implement Level 4A here
    printf("\nLevel 4 A:\n");
    fill_matrix(C, N, 1.0);
    for(int i = 1; i<N; i++){
        for(int j = i; j < N; j++){
            C[(i*N) + j] = (C[i * N + j] + C[(i-1) * N + j]);
        }
    }
    printf("Checkpoint element Serial dependency = %.2f\n", C[(6020)]);

    // TODO: Implement Level 4B here
    printf("\nLevel 4 B:\n");
    fill_matrix(C, N, 1.0);

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int nthr = omp_get_num_threads();

        int chunk = N/nthr;
        int start = id * chunk;
        int end = start + chunk;

        if(id == nthr -1){
            end = N;
        }

        for(int i = start; i<end; i++){
            for(int j = i; j < end; j++){
                C[(i*start)] = (C[i*start] + C[(i-1) * start+ j]);
            }
        }
        
    }
    printf("Checkpoint element for block scheduling = %.2f\n", C[(6020)]);
    // the output is wrong because it dependso n the previous value and the chunks are working on their own locality o they get their dependency values per chunk
    // at the boundary it resets
    //RAW
    //bklagh 

    // =====================================================
    // LEVEL 5: BARRIER SYNCHRONIZATION
    // =====================================================

    // TODO: Implement Level 5A here
    printf("\nSynchronization Part A\n");
    
    fill_matrix(A, N, 1.0);
    fill_matrix(B, N, 1.0);
    fill_matrix(C, N, 0.0);

    //start = omp_get_wtime();
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int nthr = omp_get_num_threads();

        int chunk = N/nthr;
        int start = id * chunk;
        int end = start + chunk;

        if(id == nthr -1){
            end = N;
        }

        for(int i = start; i<end; i++){
            for(int j = i; j < end; j++){
                C[(i*end) + j] = (A[i * end+ j] + B[i * end + j]);
            }
        }
        printf("Thread %d finished calculation\n", id);
        printf("Thread %d starting verification\n", id);
        
    }
    end = omp_get_wtime() - start;
    printf("Elapsed time synchro part A = %.10f\n", end);
    // TODO: Implement Level 5B here

    printf("\nSynchronization Part B\n");
    
    fill_matrix(A, N, 1.0);
    fill_matrix(B, N, 1.0);
    fill_matrix(C, N, 0.0);

    //start = omp_get_wtime();
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int nthr = omp_get_num_threads();

        int chunk = N/nthr;
        int start = id * chunk;
        int end = start + chunk;

        if(id == nthr -1){
            end = N;
        }

        for(int i = start; i<end; i++){
            for(int j = i; j < end; j++){
                C[(i*end) + j] = (A[i * end+ j] + B[i * end + j]);
            }
        }
        printf("Thread %d finished calculation\n", id);
        #pragma opm barrier 
        printf("Thread %d starting verification\n", id);
        
    }
    end = omp_get_wtime() - start;
    printf("Elapsed time synchro part B = %.10f\n", end);


    free(A);
    free(B);
    free(C);
    return 0;
}
