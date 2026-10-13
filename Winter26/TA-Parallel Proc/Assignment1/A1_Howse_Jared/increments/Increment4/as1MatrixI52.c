#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

static double avg10(const double *times) {
    double sum = 0.0;
    for (int i = 1; i <= 10; i++){
        sum += times[i];
    }
    return sum / 10.0;
}


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
        }
    }
}

void reset(double *m, int n){
    for (int i = 0; i < n*n; i++) m[i] = 0;
}

double serial(double *A, double *B, double *C, int n){
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
    return end;
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
    reset(C,n);

    printf("Starting 10 runs wait a moment...\n");
    int threads;
    #pragma omp parallel
    {
        #pragma omp single
        threads = omp_get_num_threads();
    }
    int calc = omp_get_max_threads();
    int chunk = n/ (calc * 4);
    double start, end;
    double runs[11];
    for(int z = 0; z<=10; z++){
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
        runs[z] = end;
    }
    double avg = avg10(runs);
    double serialTime = serial(A,B,C,n);
    double speedup = serialTime/avg;
    double efficiency = (speedup/(double)(threads));
    printf("Mode 1 static, size %i, # of threads %i\n", n, threads);
    printf("Mode 1 static, run time: %.5fs\n",avg);
    serialchecks(C,n);
    printf("Serial time = %.5f\n", serialTime);
    printf("speedup = %.5f\n", speedup);
    printf("efficiency = %.5f\n", efficiency);
    printf("Done mode 1 static\n");

    reset(C,n);

    printf("Starting 10 runs wait a moment...\n");
    #pragma omp parallel
    {
        #pragma omp single
        threads = omp_get_num_threads();
    }
    double runs2[11];
    for(int z = 0; z<=10; z++){
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
        runs2[z] = end;
    }
    double avg2 = avg10(runs2);
    double speedup2 = serialTime/avg2;
    double efficiency2 = (speedup2/(double)(threads));
    printf("Mode 1 dynamic, size %i, # of threads %i chunk size = %i\n", n, threads, chunk);
    printf("Mode 1 dynamic, run time: %.5fs\n",avg2);
    serialchecks(C,n);
    printf("Serial time = %.5f\n", serialTime);
    printf("speedup = %.5f\n", speedup2);
    printf("efficiency = %.5f\n", efficiency2);
    printf("Done mode 1 dynamic\n");

    
}
void mode2(double *A, double *B, double *C, int n){
    reset(C,n);
    printf("Starting 10 runs wait a moment...\n");
    int threads;
    #pragma omp parallel
    {
        #pragma omp single
        threads = omp_get_num_threads();
    }
    double start, end;
    double runs[11];
    for(int z = 0; z<=10; z++){
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
        runs[z] = end;
    }
    double avg = avg10(runs);
    double serialTime = serial(A,B,C,n);
    double speedup = serialTime/avg;
    double efficiency = (speedup/(double)(threads));
    printf("Mode 2 OMP threads + collapse, size %i, # of threads %i\n", n, threads);
    printf("Mode 2 OMP threads + collapse, run time: %.5fs\n",avg);
    serialchecks(C,n);
    printf("Serial time = %.5f\n", serialTime);
    printf("speedup = %.5f\n", speedup);
    printf("efficiency = %.5f\n", efficiency);
    printf("Done mode 2\n");
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
    printf("Starting 10 runs wait a moment...\n");
    int threads;
    #pragma omp parallel
    {
        #pragma omp single
        threads = omp_get_num_threads();
    }
    double start, end;
    double runs[11];
    for(int z = 0; z<=10; z++){
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
        runs[z] = end;
    }
    double avg = avg10(runs);
    double serialTime = serial(A,B,C,n);
    double speedup = serialTime/avg;
    double efficiency = (speedup/(double)(threads));
    printf("Mode 3 synchro A vs C, size %i, # of threads %i\n", n, threads);
    printf("Mode 3 synchro A vs C run time: %.5fs\n",avg);
    serialchecks(C,n);
    printf("Serial time = %.5f\n", serialTime);
    printf("speedup = %.5f\n", speedup);
    printf("efficiency = %.5f\n", efficiency);
    compareAC(C,n);
    printf("Done mode 3\n");
}

void mode4(double *A, double *B, double *C, int n){
    reset(C, n);
    int rownum = 16;
    printf("Starting 10 runs wait a moment...\n");
    int threads;
    #pragma omp parallel
    {
        #pragma omp single
        threads = omp_get_num_threads();
    }
    double start, end;
    double runs[11];
    for(int z = 0; z<=10; z++){
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
        runs[z] = end;
    }
    double avg = avg10(runs);
    double serialTime = serial(A,B,C,n);
    double speedup = serialTime/avg;
    double efficiency = (speedup/(double)(threads));
    printf("Mode 4 taskgroup, size %i, # of threads %i\n", n, threads);
    printf("Mode 4 taskgroup run time: %.5fs\n",avg);
    serialchecks(C,n);
    printf("Serial time = %.5f\n", serialTime);
    printf("speedup = %.5f\n", speedup);
    printf("efficiency = %.5f\n", efficiency);
    printf("Done mode 4\n");
}

void mode5(double *A, double *B, double *C, int n){
    omp_set_dynamic(0);
    omp_set_num_threads(1);
    reset(C,n);
    printf("Starting 10 runs wait a moment...\n");
    int threads;
    #pragma omp parallel
    {
        #pragma omp single
        threads = omp_get_num_threads();
    }
    double start, end;
    double runs[11];
    for(int z = 0; z<=10; z++){
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
        runs[z] = end;
    }
    double avg = avg10(runs);
    double serialTime = serial(A,B,C,n);
    double speedup = serialTime/avg;
    double efficiency = (speedup/(double)(threads));
    printf("Mode 5 SIMD only, size %i, # of threads %i\n", n, threads);
    printf("Mode 5 SIMD only run time average: %.5fs\n",avg);
    serialchecks(C,n);
    printf("Serial time = %.5f\n", serialTime);
    printf("speedup = %.5f\n", speedup);
    printf("Done mode 5\n");
}

double compare16(double time1, int n, double *A, double *B, double *C){
    reset(C,n);
    double start, end;
    double runs[11];
    for(int z = 0; z<=10; z++){
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
        runs[z] = end;
    }
    double avg = avg10(runs);
    return avg;
}

void mode6(double *A, double *B, double *C, int n){
    reset(C,n);
    printf("Starting 10 runs wait a moment...\n");
    int threads;
    #pragma omp parallel
    {
        #pragma omp single
        threads = omp_get_num_threads();
    }
    double start, end;
    double runs[11];
    for(int z = 0; z<=10; z++){
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
        runs[z] = end;
    }
    double avg = avg10(runs);
    double serialTime = serial(A,B,C,n);
    double speedup = serialTime/avg;
    double efficiency = (speedup/(double)(threads));
    printf("Mode 6 SIMD and threads, size %i, # of threads %i\n", n, threads);
    printf("Mode 6 SIMD and threads run time average: %.5fs\n",avg);
    serialchecks(C,n);
    printf("Serial time = %.5f\n", serialTime);
    printf("speedup = %.5f\n", speedup);
    printf("efficiency = %.5f\n", efficiency);
    reset(C,n);
    printf("Comparing mode 1 and 6:\n");
    double mode1Time = compare16(avg, n, A, B, C);
    printf("Mode1 = %.5f, Mode6 = %.5f\n", mode1Time, avg);
    printf("Done mode 6\n");
}


int main(int argc, char **argv)
{
    int n = atoi(argv[1]);
    int mode = atoi(argv[2]);

    omp_set_num_threads(2);

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