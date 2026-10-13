#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>


//starting to implement the matrix functionlity for this assignment we need to allocate and fill them
#define N 1024

double* alloc_matrix(int n) {
    double *m = (double*) malloc((size_t)n * (size_t)n * sizeof(double));
    if (!m) {
        printf("Allocation failed!\n");
        exit(1);
    }
    return m;
}
//starting to change implementation to work properly, try to get this to work next and verify array entries.
void fill_matrix(double *m, double *m2,int n) {
    for (int i = 0; i < n; i++){
        //value setting seems to be working with this! we access with i and j to do the deterministic filling per the assignment requirements, printing shows all indices are filled 
        for(int j = 0; j < n; j++){
            m[i*N + j] = sin(i) * cos(j) + sqrt(i + j + 1);
            m2[i*N + j] = cos(i) * sin(j) + sqrt(i + j + 2);
            //printf("indicie [%i, %i][%d]", i, j, m[i*N + j]);
           // printf("indicie [%i, %i][%d]", i, j, m2[i*N + j]);
        }
    }
}

void reset(double *m, int n){
    for (int i = 0; i < n*n; i++) m[i] = 0;
}

int main(int argc, char **argv) {
    //for setting number of threads 
    if (argc >= 2) {
        int t = atoi(argv[1]);
        if (t > 0) omp_set_num_threads(t);
    }

    double *A = alloc_matrix(N);
    double *B = alloc_matrix(N);
    double *C = alloc_matrix(N);

    //changed the filling method to accomodate deterministic filling
    fill_matrix(A, B, N);

    //Serial baseline implementation, functioning allocation and filling, C gets computing from A*B
    printf("mode 1 start:\n");
    double start, end;
    start = omp_get_wtime();
    for(int i = 0; i<N; i++){
        for(int j = 0; j < N; j++){
            double sum = 0;
            for(int z = 0; z<N; z++){
                sum += (A[i * N + z] + B[z * N + j]);
            }
            C[(i*N) + j] = sum;
        }  
    }
    double sumC = 0;
    double maxC = 0;
    double checksum= 0;

    for(int i = 0; i<N*N; i++){
        if(C[i] > maxC){
            maxC = C[i];
        }
        sumC += C[i];
        checksum += (long long)(C[i] * 1000.0) % 100000;
    }
    end = omp_get_wtime() - start;
    printf("Elapsed time serial = %.10f\n", end);
    printf("SumC serial value = %f\n", sumC);
    printf("MaxC serial value = %f\n", maxC);
    printf("checksum serial value = %f\n", checksum);
    //----------------------------------------------------
    printf("Mode 2 start:\n");
    //Mode 1 Starts Below, assuming we dont need to overwrite C values with zeroes we can just re multiply A and B and put into C should not matter at all?
    reset(C, N);
    sumC = 0;
    maxC = 0;
    checksum= 0;
    //fixed errors on mismatched serial and parallel values
    start = omp_get_wtime();
    #pragma omp parallel for collapse(2) shared(A,B,C) schedule(static)
    for(int i = 0; i<N; i++){
        for(int j = 0; j < N; j++){
            double sum = 0;
            for(int k = 0; k<N; k++){
                sum += A[i * N + k] * B[k * N + j];
            }
            C[(i*N) + j] = sum;
        }  
    }
    sumC = 0;
    maxC = 0;
    checksum= 0;
    //still checking serially for these values changing after full optimization of main multiplication
    for(int i = 0; i<N*N; i++){
        if(C[i] > maxC){
            maxC = C[i];
        }
        sumC += C[i];
        checksum += (long long)(C[i] * 1000.0) % 100000;
    }
    end = omp_get_wtime() - start;
    printf("Elapsed time parallel = %.10f\n", end);
    printf("SumC parallel value = %f\n", sumC);
    printf("MaxC parallel value = %f\n", maxC);
    printf("checksum parallel value = %f\n", checksum);

    sumC = 0;
    maxC = 0;
    checksum= 0;
    //chunk version
    int chunk = 4;
    start = omp_get_wtime();
    #pragma omp parallel for collapse(2) shared(A,B,C) schedule(dynamic, chunk)
    for(int i = 0; i<N; i++){
        for(int j = 0; j < N; j++){
            double sum = 0;
            for(int z = 0; z<N; z++){
                sum += (A[i * N + z] + B[z * N + j]);
            }
            C[(i*N) + j] = sum;
        }  
    }
    for(int i = 0; i<N*N; i++){
        if(C[i] > maxC){
            maxC = C[i];
        }
        sumC += C[i];
        checksum += (long long)(C[i] * 1000.0) % 100000;
    }
    end = omp_get_wtime() - start;
    //testing parallelization on serial calculators before trying parallel with synchronizations
    printf("Elapsed time parallel chunk = %.10f\n", end);
    printf("SumC parallel value = %f\n", sumC);
    printf("MaxC parallel value = %f\n", maxC);
    printf("checksum parallel value = %f\n", checksum);
}