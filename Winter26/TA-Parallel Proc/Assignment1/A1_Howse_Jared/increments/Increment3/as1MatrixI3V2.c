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
    printf("\nLevel 1:\n");
    double start, end;
    start = omp_get_wtime();
    int sum = 0;
    //need to have triple loop multiplicaition and then compute SumC, MaxC, and Checksum serially, need to change to have 3 loops and work that way 
    for(int i = 0; i<N; i++){
        // this should be working matrix multiplication
        for(int j = 0; j < N; j++){
            sum = 0;
            //unsure if we really need to set this value to zero before we fill it???
            C[(i*N) + j] = 0;
            for(int z = 0; z<N; z++){
                sum += (A[i * N + z] + B[z * N + j]);
            }
            C[(i*N) + j] += sum;
        }  
    }
    int sumC = 0;
    int maxC = 0;
    int checksum= 0;
    //sumC serial
    // for(int i = 0; i<N*N; i++){
    //     sumC += C[i];
    // }
    //maxC serial
    for(int i = 0; i<N*N; i++){
        if(C[i] > maxC){
            maxC = C[i];
        }
        sumC += C[i];
        checksum += (long long)(C[i] * 1000.0) % 100000;
    }
    //checksum serial
    //  for(int i = 0; i<N*N; i++){
    //     checksum += (long long)(C[i] * 1000.0) % 100000;
    // }
    //now that we have C we need to calculate the other factors in C -> SumC, MaxC, and Checksum, all serially.
    end = omp_get_wtime() - start;
    printf("Elapsed time serial = %.10f\n", end);
    printf("SumC serial value = %i\n", sumC);
    printf("MaxC serial value = %i\n", maxC);
    printf("checksum serial value = %i\n", checksum);
    //removed parallel version for now, this is the serial version in a should be completed version!
}