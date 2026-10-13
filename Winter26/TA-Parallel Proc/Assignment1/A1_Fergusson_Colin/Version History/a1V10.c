#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

void mode0(double* A, double* B, int N, double* sumC, double* maxC, double* checksum){
    double* C = (double*)malloc(sizeof(double)*N*N);

    for(int i = 0; i<N; i++){
        for(int j = 0; j<N; j++){
            C[i*N+j] = 0;
            for(int k = 0; k<N; k++){
                C[i*N+j] += A[i*N+k]*B[k*N+j];
            }
        }
    }
    *maxC = C[0];
    for(int i = 0; i<N; i++){
        for(int j = 0; j<N; j++){
            *sumC += C[i*N+j];
            if(C[i*N+j]>*maxC){
                *maxC = C[i*N+j];
            }
            *checksum += (long long)(C[i*N + j] * 1000.0) % 100000;
        }
    }
    free(C);
}

void mode1(double* A, double* B, int N, double* sumC, double* maxC, double* checksum){
    double* C = (double*)malloc(sizeof(double)*N*N);
    int i,j,k;
    double sumCl = 0, maxCl = 0, checksuml = 0;

    #pragma omp parallel for private(i,j,k) shared(A,B,C) schedule(static)
    for(i = 0; i<N; i++){
        for(j = 0; j<N; j++){
            C[i*N+j] = 0;
            for(k = 0; k<N; k++){
                C[i*N+j] += A[i*N+k]*B[k*N+j];
            }
        }
    }
    maxCl = C[0];
    #pragma omp parallel for private(i,j) shared(C) schedule(dynamic, 16) reduction(+:sumCl) reduction(max:maxCl) reduction(+:checksuml)
    for(i = 0; i<N; i++){
        for(j = 0; j<N; j++){
            sumCl += C[i*N+j];
            if(C[i*N+j]>*maxC){
                maxCl = C[i*N+j];
            }
            checksuml += (long long)(C[i*N + j] * 1000.0) % 100000;
        }
    }

    *sumC = sumCl;
    *maxC = maxCl;
    *checksum = checksuml;
    
    free(C);
}

void mode2(double* A, double* B, int N, double* sumC, double* maxC, double* checksum){
    double* C = (double*)malloc(sizeof(double)*N*N);
    int i,j,k;
    double sumCl = 0, maxCl = 0, checksuml = 0;

    #pragma omp parallel for collapse(2) private(i,j,k) shared(A,B,C) schedule(static)
    for(i = 0; i<N; i++){
        for(j = 0; j<N; j++){
            C[i*N+j] = 0;
            for(k = 0; k<N; k++){
                C[i*N+j] += A[i*N+k]*B[k*N+j];
            }
        }
    }
    maxCl = C[0];
    #pragma omp parallel for collapse(2) private(i,j) shared(C) schedule(dynamic, 16) reduction(+:sumCl) reduction(max:maxCl) reduction(+:checksuml)
    for(i = 0; i<N; i++){
        for(j = 0; j<N; j++){
            sumCl += C[i*N+j];
            if(C[i*N+j]>*maxC){
                maxCl = C[i*N+j];
            }
            checksuml += (long long)(C[i*N + j] * 1000.0) % 100000;
        }
    }

    *sumC = sumCl;
    *maxC = maxCl;
    *checksum = checksuml;
    
    free(C);
}

void mode3A(double* A, double* B, int N, double* sumC, double* maxC, double* checksum){
    double* C = (double*)malloc(sizeof(double)*N*N);
    int i,j,k;
    double sumCl = 0, maxCl = 0, checksuml = 0;

    #pragma omp parallel for collapse(2) private(i,j,k) shared(A,B,C) schedule(static)
    for(i = 0; i<N; i++){
        for(j = 0; j<N; j++){
            C[i*N+j] = 0;
            for(k = 0; k<N; k++){
                C[i*N+j] += A[i*N+k]*B[k*N+j];
            }
        }
    }
    maxCl = C[0];
    #pragma omp parallel for collapse(2) private(i,j) shared(C) schedule(dynamic, 16) reduction(+:sumCl) reduction(max:maxCl) reduction(+:checksuml)
    for(i = 0; i<N; i++){
        for(j = 0; j<N; j++){
            sumCl += C[i*N+j];
            if(C[i*N+j]>*maxC){
                maxCl = C[i*N+j];
            }
        }
    }

    #pragma omp parallel for collapse(2) private(i,j) shared(C)
    for(i = 0; i<N; i++){
        for(j = 0; j<N; j++){
            #pragma omp atomic
            checksuml += (long long)(C[i*N + j] * 1000.0) % 100000;
        }
    }

    *sumC = sumCl;
    *maxC = maxCl;
    *checksum = checksuml;
    
    free(C);
}

void mode3B(double* A, double* B, int N, double* sumC, double* maxC, double* checksum){
    double* C = (double*)malloc(sizeof(double)*N*N);
    int i,j,k;
    double sumCl = 0, maxCl = 0, checksuml = 0;

    #pragma omp parallel for collapse(2) private(i,j,k) shared(A,B,C) schedule(static)
    for(i = 0; i<N; i++){
        for(j = 0; j<N; j++){
            C[i*N+j] = 0;
            for(k = 0; k<N; k++){
                C[i*N+j] += A[i*N+k]*B[k*N+j];
            }
        }
    }
    maxCl = C[0];
    #pragma omp parallel for collapse(2) private(i,j) shared(C) schedule(dynamic, 16) reduction(+:sumCl) reduction(max:maxCl) reduction(+:checksuml)
    for(i = 0; i<N; i++){
        for(j = 0; j<N; j++){
            sumCl += C[i*N+j];
            if(C[i*N+j]>*maxC){
                maxCl = C[i*N+j];
            }
        }
    }

    #pragma omp parallel for collapse(2) private(i,j) shared(C)
    for(i = 0; i<N; i++){
        for(j = 0; j<N; j++){
            #pragma omp critical
            {
                checksuml += (long long)(C[i*N + j] * 1000.0) % 100000;
            }
        }
    }

    *sumC = sumCl;
    *maxC = maxCl;
    *checksum = checksuml;
    
    free(C);
}

void mode4(double* A, double* B, int N, double* sumC, double* maxC, double* checksum){
    double* C = (double*)malloc(sizeof(double)*N*N);
    int i,j,k,l;
    double sumCl = 0, maxCl = 0, checksuml = 0;
    int start,end;

    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for(l = 0; l<N; l+=16){
                    start = l;
                    end = start+16;

                    #pragma omp task shared(A,B,C) private(i,j,k)
                    {
                        for(i = start; i<end; i++){
                            for(j = 0; j<N; j++){
                                C[i*N+j] = 0;
                                for(k = 0; k<N; k++){
                                    C[i*N+j] += A[i*N+k]*B[k*N+j];
                                }
                            }
                        }
                    }
                }
            }
            maxCl = C[0];
            #pragma omp taskgroup
            {
                #pragma omp task shared(C) private(i,j) 
                {
                    for(i = 0; i<N; i++){
                        for(j = 0; j<N; j++){
                            sumCl += C[i*N+j];
                            if(C[i*N+j]>*maxC){
                                maxCl = C[i*N+j];
                            }
                            checksuml += (long long)(C[i*N + j] * 1000.0) % 100000;
                        }
                    }
                }
            }
        }
    }

    *sumC = sumCl;
    *maxC = maxCl;
    *checksum = checksuml;
    
    free(C);
}

void mode5(double* A, double* B, int N, double* sumC, double* maxC, double* checksum){
    double* C = (double*)malloc(sizeof(double)*N*N);
    int i,j,k;
    double sumCl = 0, maxCl = 0, checksuml = 0;

    for(i = 0; i<N; i++){
        for(j = 0; j<N; j++){
            C[i*N+j] = 0;
            #pragma omp simd
            for(k = 0; k<N; k++){
                C[i*N+j] += A[i*N+k]*B[k*N+j];
            }
        }
    }
    maxCl = C[0];
    for(i = 0; i<N; i++){
        #pragma omp simd
        for(j = 0; j<N; j++){
            sumCl += C[i*N+j];
            if(C[i*N+j]>*maxC){
                maxCl = C[i*N+j];
            }
            checksuml += (long long)(C[i*N + j] * 1000.0) % 100000;
        }
    }

    *sumC = sumCl;
    *maxC = maxCl;
    *checksum = checksuml;
    
    free(C);
}

void mode6(double* A, double* B, int N, double* sumC, double* maxC, double* checksum){
    double* C = (double*)malloc(sizeof(double)*N*N);
    int i,j,k;
    double sumCl = 0, maxCl = 0, checksuml = 0;

    #pragma omp parallel for simd
    for(i = 0; i<N; i++){
        for(j = 0; j<N; j++){
            C[i*N+j] = 0;
            for(k = 0; k<N; k++){
                C[i*N+j] += A[i*N+k]*B[k*N+j];
            }
        }
    }
    maxCl = C[0];
    #pragma omp parallel for simd reduction(+:sumCl) reduction(max:maxCl) reduction(+:checksuml)
    for(i = 0; i<N; i++){
        for(j = 0; j<N; j++){
            sumCl += C[i*N+j];
            if(C[i*N+j]>*maxC){
                maxCl = C[i*N+j];
            }
            checksuml += (long long)(C[i*N + j] * 1000.0) % 100000;
        }
    }

    *sumC = sumCl;
    *maxC = maxCl;
    *checksum = checksuml;
    
    free(C);
}


int main(int argc, char **argv){
    if(argc != 3){
        printf("Please include the size of the matrices and the mode as command line arguments\n");
        return 1;
    }
    int N = atoi(argv[1]);
    char* mode = argv[2];
    int threads[] = {1,2,4,8,12,16,20};
    double time;

    double* A = (double*)malloc(sizeof(double)*N*N);
    double* B = (double*)malloc(sizeof(double)*N*N);
    double sumC = 0, maxC = 0, checksum = 0;
    double start, end;

    for(int i = 0; i<N; i++){
        for(int j = 0; j<N; j++){
            A[i*N+j] = sin(i) * cos(j) + sqrt(i+j+1);
            B[i*N+j] = cos(i) * sin(j) + sqrt(i+j+2);
        }
    }

    for(int j = 0; j<7; j++){
        omp_set_num_threads(threads[j]);
        time = 0;

        switch(mode[0]){
            case '0':
                for(int i = 0; i<10; i++){
                    start = omp_get_wtime();
                    mode0(A, B, N, &sumC, &maxC, &checksum);
                    end = omp_get_wtime();
                    if(i!=0){
                        time += end-start;
                    }
                }
                time = time/9;
                break;
            case '1':
                for(int i = 0; i<10; i++){
                    start = omp_get_wtime();
                    mode1(A, B, N, &sumC, &maxC, &checksum);
                    end = omp_get_wtime();
                    if(i!=0){
                        time += end-start;
                    }
                }
                time = time/9;
                break;
            case '2':
                for(int i = 0; i<10; i++){
                    start = omp_get_wtime();
                    mode2(A, B, N, &sumC, &maxC, &checksum);
                    end = omp_get_wtime();
                    if(i!=0){
                        time += end-start;
                    }
                }
                time = time/9;
                break;
            case '3':
                if(mode[1]=='A'){
                    for(int i = 0; i<10; i++){
                        start = omp_get_wtime();
                        mode3A(A, B, N, &sumC, &maxC, &checksum);
                        end = omp_get_wtime();
                        if(i!=0){
                            time += end-start;
                        }
                    }
                    time = time/9;
                }
                else if(mode[1]=='B'){
                    for(int i = 0; i<10; i++){
                        start = omp_get_wtime();
                        mode3B(A, B, N, &sumC, &maxC, &checksum);
                        end = omp_get_wtime();
                        if(i!=0){
                            time += end-start;
                        }
                    }
                    time = time/9;
                }
                else{
                    printf("Invalid Mode\n");
                    free(A);
                    free(B);
                    return 1;
                }
                break;
            case '4':
                for(int i = 0; i<10; i++){
                    start = omp_get_wtime();
                    mode4(A, B, N, &sumC, &maxC, &checksum);
                    end = omp_get_wtime();
                    if(i!=0){
                        time += end-start;
                    }
                }
                time = time/9;
                break;
            case '5':
                for(int i = 0; i<10; i++){
                    start = omp_get_wtime();
                    mode5(A, B, N, &sumC, &maxC, &checksum);
                    end = omp_get_wtime();
                    if(i!=0){
                        time += end-start;
                    }
                }
                time = time/9;
                break;
            case '6':
                for(int i = 0; i<10; i++){
                    start = omp_get_wtime();
                    mode6(A, B, N, &sumC, &maxC, &checksum);
                    end = omp_get_wtime();
                    if(i!=0){
                        time += end-start;
                    }
                }
                time = time/9;
                break;
            default:
                printf("Invalid Mode\n");
                free(A);
                free(B);
                return 1;
        }
        printf("N: %d\n", N);
        printf("Mode: %s\n", mode);
        printf("Threads: %d\n", threads[j]);
        printf("Average time: %f seconds\n", time);
        printf("SumC: %f\n", sumC);
        printf("MaxC: %f\n", maxC);
        printf("Checksum: %f\n\n", checksum);
    }

    free(A);
    free(B);
    return 0;
}