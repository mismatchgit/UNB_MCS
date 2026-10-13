#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


//starting to implement the matrix functionlity for this assignment we need to allocate and fill them
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
    //Serial baseline 
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

    #pragma omp parallel for
    for(int i = 0; i<N; i++){
        for(int j = 0; j < N; j++){
            C[(i*N) + j] = (A[i * N + j] + B[i * N + j]);
        }
        
    }
}