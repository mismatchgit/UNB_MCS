#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

double* alloc_matrix(int n) {
    double *m = (double*) malloc((size_t)n * (size_t)n * sizeof(double));
    if (!m) {
        printf("Allocation failed!\n");
        exit(1);
    }
    return m;
}

void fill_matrix(double *m, double *m2,int n) {
    for (int i = 0; i < n; i++){
    
        for(int j = 0; j < n; j++){
            m[i*n + j] = sin(i) * cos(j) + sqrt(i + j + 1);
            m2[i*n + j] = cos(i) * sin(j) + sqrt(i + j + 2);
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

void atomicchecks(double *C, int n){
    double sumC = 0.0;
    double maxC = -INFINITY;
    long long checksum = 0;
    #pragma omp parallel for shared(C,n) reduction(+:sumC) reduction(max:maxC)
    for(int i = 0; i< n*n; i++){
        double temp = C[i];
        sumC += temp;
        if(temp > maxC){
            maxC = temp;
        }
        long long inc = (long long)(C[i] * 1000.0) % 100000;
        #pragma omp atomic update
        checksum += inc;
    }
    printf("Sum C atomic = %f\n", sumC);
    printf("Max C atomic = %f\n", maxC);
    printf("Checksum atomic = %lld\n", checksum);
}

void criticalchecks(double *C, int n){
    double sumC = 0.0;
    double maxC = -INFINITY;
    long long checksum = 0;
    #pragma omp parallel for shared(C,n) reduction(+:sumC) reduction(max:maxC)
    for(int i = 0; i< n*n; i++){
        double temp = C[i];
        sumC += temp;
        if(temp > maxC){
            maxC = temp;
        }
        long long inc = (long long)(C[i] * 1000.0) % 100000;
        #pragma omp critical(checksumcritical)
        {
            checksum += inc;
        }
    }
    printf("Sum C critical = %f\n", sumC);
    printf("Max C critical = %f\n", maxC);
    printf("Checksum critical = %lld\n", checksum);
}

void mode1(double *A, double *B, double *C, int n){
    int chunk = 4;
    reset(C, n);
    double start, end;
    //Mode 1 static
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

    serialchecks(C, n);
    reset(C,n);

    //Mode 1 dynamic
    start = omp_get_wtime();
    #pragma omp parallel for shared(A,B,C) schedule(dynamic, chunk)
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

    serialchecks(C,n);
    reset(C,n);
}
void mode2(double *A, double *B, double *C, int n){
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
    serialchecks(C,n);
    reset(C,n);
}


void compareAC(double *C, int n){
    printf("\nComparing Atomic and Critical for Mode3\n");
    double start, end, atomic, critical;
    start = omp_get_wtime();
    atomicchecks(C,n);
    end = omp_get_wtime() - start;
    atomic = end;

    start = omp_get_wtime();
    criticalchecks(C,n);
    end = omp_get_wtime() - start;
    critical = end;
    printf("\nAtomic time = %.5f VS Critical time = %.5f\n", atomic,critical);
}


void mode3(double *A, double *B, double *C, int n){
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
    printf("multiplication time mode 3: %.5fs\n", end);
    compareAC(C,n);
}

void mode4(double *A, double *B, double *C, int n){
    reset(C, n);
    double start, end;
    int rownum = 16;
    int threads;

    start = omp_get_wtime();
    #pragma omp parallel shared(A,B,C,n, threads, rownum)
    {
        #pragma omp single
        {
            threads = omp_get_num_threads();

            #pragma omp taskgroup
            {
                for(int start = 0; start<n; start+= rownum){
                    int end = start + rownum;
                    if(end > n){
                        end = n;
                    }
                    //this set up basically lets us do < n so each section does the normal code
                    #pragma omp task firstprivate(start, end) shared(A,B,C,n)
                    {
                        for(int i = start; i<end; i++){
                            for(int j = 0; j < n; j++){
                                double sum = 0;
                                for(int k = 0; k<n; k++){
                                    sum += A[i * n + k] * B[k * n + j];
                                }
                                C[(i*n) + j] = sum;
                            }  
                        }
                    }
                }
            }
        }
    }
    end = omp_get_wtime() - start;
    printf("mode 4 # of threads: %i\n", threads);
    printf("multiplication time mode 4: %.5fs\n", end);
}

void mode5(double *A, double *B, double *C, int n){
    omp_set_dynamic(0);
    omp_set_num_threads(1);
    reset(C,n);

    double start, end;

    start = omp_get_wtime();
    for(int i = 0; i<n; i++){
        for(int j = 0; j < n; j++){
            double sum = 0;
            #pragma omp simd reduction(+:sum)
            for(int k = 0; k<n; k++){
                sum += A[i * n + k] * B[k * n + j];
            }
            C[(i*n) + j] = sum;
        }  
    }
    end = omp_get_wtime() - start;
    printf("Mode 5 SIMD only ime: %.5f\n", end);
    serialchecks(C,n);
    printf("Done mode 5\n");
}

void mode6(double *A, double *B, double *C, int n){
    reset(C,n);

    double start, end;

    start = omp_get_wtime();

    #pragma omp parallel for collapse(2) shared(A,B,C,n) schedule(static)
    for(int i = 0; i<n; i++){
        for(int j = 0; j < n; j++){
            double sum = 0;
            #pragma omp simd reduction(+:sum)
            for(int k = 0; k<n; k++){
                sum += A[i * n + k] * B[k * n + j];
            }
            C[(i*n) + j] = sum;
        }  
    }
    end = omp_get_wtime() - start;
    printf("Mode 6 SIMD and threads: %.5f\n", end);
    serialchecks(C,n);
    printf("Done mode 6\n");
}


int main(int argc, char **argv)
{
    int n = atoi(argv[1]);
    int mode = atoi(argv[2]);

    omp_set_num_threads(8);

    double *A = alloc_matrix(n);
    double *B = alloc_matrix(n);
    double *C = alloc_matrix(n);

    fill_matrix(A, B, n);

    switch (mode) {
        case 0: 
            serial(A, B, C, n); 
            serialchecks(C, n); 
            break;
        case 1: 
            mode1(A, B, C, n); 
            break;
        case 2: 
            mode2(A, B, C, n); 
            break;
        case 3: 
            mode3(A, B, C, n); 
            break;
        case 4: 
            mode4(A, B, C, n); 
            break;
        case 5: 
            mode5(A, B, C, n); 
            break;
        case 6: 
            mode6(A, B, C, n); 
            break;
        default:
            printf("Enter a valid mode!");
            break;
    }

    free(A); 
    free(B); 
    free(C);
    return 0;
}

