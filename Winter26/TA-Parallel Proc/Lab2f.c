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

    // =====================================================
    // LEVEL 2: MANUAL BLOCK DECOMPOSITION
    // =====================================================

    // TODO: Implement Level 2A here

    // TODO: Implement Level 2C here

    // =====================================================
    // LEVEL 3: CYCLIC DECOMPOSITION
    // =====================================================

    // TODO: Implement Level 3 here

    // =====================================================
    // LEVEL 4: DEPENDENCY TRAP
    // =====================================================

    // TODO: Implement Level 4A here

    // TODO: Implement Level 4B here
    // =====================================================
    // LEVEL 5: BARRIER SYNCHRONIZATION
    // =====================================================

    // TODO: Implement Level 5A here

    // TODO: Implement Level 5B here

    free(A);
    free(B);
    free(C);
    return 0;
}
