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

int main(int argc, char **argv){
    if(argc != 3){
        printf("Please include the size of the matrices and the mode as command line arguments\n");
        return 1;
    }
    int N = atoi(argv[1]);
    char* mode = argv[2];
    int threads = 8;
    omp_set_num_threads(threads);

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

    switch(mode[0]){
        case '0':
            start = omp_get_wtime();
            mode0(A, B, N, &sumC, &maxC, &checksum);
            end = omp_get_wtime();
            break;
        case '1':
            start = omp_get_wtime();
            mode1(A, B, N, &sumC, &maxC, &checksum);
            end = omp_get_wtime();
            break;
        case '2':
            start = omp_get_wtime();
            mode2(A, B, N, &sumC, &maxC, &checksum);
            end = omp_get_wtime();
            break;
        case '3':
            if(mode[1]=='A'){

            }
            else if(mode[1]=='B'){
                
            }
            else{
                printf("Invalid Mode\n");
                free(A);
                free(B);
                return 1;
            }
            break;
        case '4':
            break;
        case '5':
            break;
        case '6':
            break;
        default:
            printf("Invalid Mode\n");
            free(A);
            free(B);
            return 1;
    }
    printf("N: %d\n", N);
    printf("Mode: %s\n", mode);
    printf("Threads: %d\n", threads);
    printf("Time: %f seconds\n", end-start);
    printf("SumC: %f\n", sumC);
    printf("MaxC: %f\n", maxC);
    printf("Checksum: %f\n", checksum);

    free(A);
    free(B);
}