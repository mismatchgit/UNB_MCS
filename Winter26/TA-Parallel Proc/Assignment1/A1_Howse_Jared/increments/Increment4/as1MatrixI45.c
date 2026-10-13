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

void serial(double *A, double *B, double *C, int n){
    printf("mode 1 start:\n");
    double start, end;
    start = omp_get_wtime();
    for(int i = 0; i<n; i++){
        for(int j = 0; j < n; j++){
            double sum = 0;
            for(int k = 0; k<n; k++){
                sum += A[i * n + k] * B[k * n + j];
            }
            C[(i*n) + j] = sum;
        }  
    }
    end = omp_get_wtime() - start;
}

void serialchecks(double *C, int n){
    double sumC = 0.0;
    double maxC = -INFINITY;
    long long checksum = 0;
    for(int i = 0; i<n*n; i++){
        if(C[i] > maxC){
            maxC = C[i];
        }
        sumC += C[i];
        checksum += (long long)(C[i] * 1000.0) % 100000;
    }
    printf("Sum C = %f\n", sumC);
    printf("Max C = %f\n", maxC);
    printf("Checksum = %lld\n", checksum);
}

void mode1(double *A, double *B, double *C, int n){
    int chunk = 4;
    reset(C, n);
    double start, end;
    //Mode 1 static
    start = omp_get_wtime();
    #pragma omp parallel for shared(A,B,C) schedule(static)
    for(int i = 0; i<N; i++){
        for(int j = 0; j < n; j++){
            double sum = 0;
            for(int k = 0; k<n; k++){
                sum += A[i * n + k] * B[k * n + j];
            }
            C[(i*n) + j] = sum;
        }  
    }
    end = omp_get_wtime() - start;

    //Mode 1 dynamic
    start = omp_get_wtime();
    #pragma omp parallel for shared(A,B,C) schedule(static)
    for(int i = 0; i<n; i++){
        for(int j = 0; j < n; j++){
            double sum = 0;
            for(int k = 0; k<n; k++){
                sum += A[i * n + k] * B[k * n + j];
            }
            C[(i*n) + j] = sum;
        }  
    }
    end = omp_get_wtime() - start;
}
//starting to implement proper method logic for the assignment requirements letting us call modes from execution
void mode2(double *A, double *B, double *C, int n){
    int chunk = 4;
    reset(C, n);
    double start, end;
    start = omp_get_wtime();
    #pragma omp parallel for collapse(2) shared(A,B,C) schedule(static)
    for(int i = 0; i<n; i++){
        for(int j = 0; j < n; j++){
            double sum = 0;
            for(int k = 0; k<n; k++){
                sum += A[i * n + k] * B[k * n + j];
            }
            C[(i*n) + j] = sum;
        }  
    }
    end = omp_get_wtime() - start;
}

void mode3(){

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

    fill_matrix(A,B, N);

    mode1(A,B,C, N);
    serialchecks(C,N);
}