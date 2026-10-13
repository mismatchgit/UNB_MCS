#include <limits.h>
#include <math.h>
#include <omp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    SERIAL = 1,
    WORK_SHARING = 2,
    COLLAPSE = 3,
    CHECKSUM_ATOMIC = 4,
    CHECKSUM_CRITICAL = 5,
    TASKS = 6,
    SIMD_SINGLE_THREADED = 7,
    SIMD_MULTI_THREADED = 8
} MODE_T;

MODE_T get_type(char *arg);

void populate_matrixes(const size_t N, double *A, double *B);

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Error: Please define N and mode, ie: ./run <N> <Mode>\n");
        exit(EXIT_FAILURE);
    }

    const size_t N = atoi(argv[1]);

    if (argc == 4){
        int thread_count = atoi(argv[3]);
        if (thread_count <= 0) {
            fprintf(stderr, "Error: thread count cannot be smaller than or equal to 0\n");
            exit(EXIT_FAILURE);
        }
        omp_set_num_threads(thread_count);
    }

    double *A, *B;
    A = malloc(sizeof(double) * N * N);
    B = malloc(sizeof(double) * N * N);
    populate_matrixes(N, A, B);

    printf("Max Thread Num: %d, ", omp_get_max_threads());


    switch (get_type(argv[2])) {
        case SERIAL: {
            printf("%ld SERIAL\n", N);
            double *C = malloc(sizeof(double) * N * N);
            if (C == NULL) {
                fprintf(stderr, "Error: Failed to allocate memory\n");
                exit(EXIT_FAILURE);
            }

            double start = omp_get_wtime();

            for (int i=0; i<N; i++){
                for (int j=0; j<N; j++){
                    C[i*N + j] = 0.0;
                    for (int k=0; k<N;k++){
                        C[i*N + j] += A[i*N + k] * B[k*N + j];
                    }
                }
            }

            double calc_end = omp_get_wtime();

            double sumC = 0.0, maxC = -9999, checksum = 0.0;

            for (int i=0; i<N; i++){
                for (int j=0;j<N; j++){
                    double value = C[i*N + j];
                    sumC += value;
                    if (maxC < value){
                        maxC = value;
                    }
                    checksum += (long long)(C[i*N + j]*1000.0)%100000;
                }
            }

            double end = omp_get_wtime();

            printf("Calculation time, Total Time, Checksum, SumC, MaxC\n");
            printf("%10lf %10lf %10lf %10lf %10lf\n", calc_end-start, end-start, checksum, sumC, maxC);

            // printf("Calculation time %f seconds\n", calc_end - start);
            // printf("Total time %f seconds\n", end - start);
            // printf("Checksum: %lf\n", checksum);
            // printf("Sum C: %lf\n", sumC);
            // printf("Max C: %lf\n", maxC);

            free(A);
            free(B);
            free(C);
            break;
        }
        case WORK_SHARING: {
            printf("%ld WORK SHARING\n", N);
            double *C = malloc(sizeof(double) * N * N);
            if (C == NULL) {
                fprintf(stderr, "Error: Failed to allocate memory\n");
                exit(EXIT_FAILURE);
            }

            double start = omp_get_wtime();

            double calc_start = omp_get_wtime();

            int i;
#ifdef DYNAMIC_SCHEDULE
            int chunk = N/(omp_get_max_threads()*4);
            if (chunk < 1)
                chunk = 1;
            #pragma omp parallel for default(none) shared(A, B, C, N) private(i) schedule(dynamic, chunk)
#endif
#ifndef DYNAMIC_SCHEDULE
            #pragma omp parallel for default(none) shared(A, B, C, N) private(i) schedule(static)
#endif
            for (i=0; i<N; i++){
                for (int j=0; j<N; j++){
                    C[i*N + j] = 0.0;
                    for (int k=0; k<N;k++){
                        C[i*N + j] += A[i*N + k] * B[k*N + j];
                    }
                }
            }

            double calc_end = omp_get_wtime();

            double sumC = 0.0, maxC = -9999, checksum = 0.0;

            for (int i=0; i<N; i++){
                for (int j=0;j<N; j++){
                    double value = C[i*N + j];
                    sumC += value;
                    if (maxC < value){
                        maxC = value;
                    }
                    checksum += (long long)(C[i*N + j]*1000.0)%100000;
                }
            }

            double end = omp_get_wtime();

            printf("Calculation time, Total Time, Checksum, SumC, MaxC\n");
            printf("%10lf %10lf %10lf %10lf %10lf\n", calc_end-start, end-start, checksum, sumC, maxC);

// #ifdef DYNAMIC_SCHEDULE
//             printf("Calculation time with schedule(dynamic, chunk) %f seconds\n", calc_end - calc_start);
// #endif
// #ifndef DYNAMIC_SCHEDULE
//             printf("Calculation time with schedule(static) %f seconds\n", calc_end - calc_start);
// #endif
//             printf("Total time %f seconds\n", end - start);
//             printf("Checksum: %lf\n", checksum);
//             printf("Sum C: %lf\n", sumC);
//             printf("Max C: %lf\n", maxC);

            free(A);
            free(B);
            free(C);
            break;
        }
        case COLLAPSE: {
            printf("%ld COLLAPSE\n", N);
            double *C = malloc(sizeof(double) * N * N);
            if (C == NULL) {
                fprintf(stderr, "Error: Failed to allocate memory\n");
                exit(EXIT_FAILURE);
            }

            double start = omp_get_wtime();

            #pragma omp parallel for collapse(2) default(none) shared(A, B, C, N) schedule(static)
            for (int i=0; i<N; i++){
                for (int j=0; j<N; j++){
                    C[i*N + j] = 0.0;
                    for (int k=0; k<N;k++){
                        C[i*N + j] += A[i*N + k] * B[k*N + j];
                    }
                }
            }

            double calc_end = omp_get_wtime();

            double sumC = 0.0, maxC = -99999, checksum = 0.0;

            for (int i=0; i<N; i++){
                for (int j=0;j<N; j++){
                    double value = C[i*N + j];
                    sumC += value;
                    if (maxC < value){
                        maxC = value;
                    }
                    checksum += (long long)(C[i*N + j]*1000.0)%100000;
                }
            }

            double end = omp_get_wtime();

            printf("Calculation time, Total Time, Checksum, SumC, MaxC\n");
            printf("%10lf %10lf %10lf %10lf %10lf\n", calc_end-start, end-start, checksum, sumC, maxC);

            // printf("Calculation time %f seconds\n", calc_end - start);
            // printf("Total time %f seconds\n", end - start);
            // printf("Checksum: %lf\n", checksum);
            // printf("Sum C: %lf\n", sumC);
            // printf("Max C: %lf\n", maxC);

            free(A);
            free(B);
            free(C);

            break;
        }
        case CHECKSUM_ATOMIC: {
            printf("%ld CHECKSUM ATOMIC\n", N);
            double *C = malloc(sizeof(double) * N * N);
            if (C == NULL) {
                fprintf(stderr, "Error: Failed to allocate memory\n");
                exit(EXIT_FAILURE);
            }

            double start = omp_get_wtime();
            // #pragma omp parallel for collapse(2) default(none) shared(A, B, C, N) schedule(static)
            #pragma omp parallel for default(none) shared(A, B, C, N) schedule(static)
            for (int i=0; i<N; i++){
                for (int j=0; j<N; j++){
                    C[i*N + j] = 0.0;
                    for (int k=0; k<N;k++){
                        C[i*N + j] += A[i*N + k] * B[k*N + j];
                    }
                }
            }
            double calc_end = omp_get_wtime();

            double checksum = 0.0, sumC = 0.0, maxC = -99999;

            #pragma omp parallel for default(none) shared(C, N, checksum, sumC, maxC) schedule(static)
            // #pragma omp parallel for collapse(2) shared(C, N, checksum, sumC, maxC)
            for (int i=0; i<N; i++){
                for (int j=0;j<N; j++){
                    double value = C[i*N + j];

                    #pragma omp atomic update
                    sumC += value;

                    #pragma omp compare
                    if (maxC < value) maxC = value;

                    #pragma omp atomic update
                    checksum += (long long)(C[i*N + j]*1000.0)%100000;
                }
            }
            double end = omp_get_wtime();

            printf("Calculation time, Total Time, Checksum, SumC, MaxC\n");
            printf("%10lf %10lf %10lf %10lf %10lf\n", calc_end-start, end-start, checksum, sumC, maxC);

            // printf("Calculation time %f seconds\n", calc_end - start);
            // printf("Checksum calculation time %f seconds\n", end - calc_end);
            // printf("Checksum: %lf\n", checksum);
            // printf("Sum C: %lf\n", sumC);
            // printf("Max C: %lf\n", maxC);

            free(A);
            free(B);
            free(C);
            break;
        }
        case CHECKSUM_CRITICAL: {
            printf("%ld CHECKSUM ATOMIC\n", N);
            double *C = malloc(sizeof(double) * N * N);
            if (C == NULL) {
                fprintf(stderr, "Error: Failed to allocate memory\n");
                exit(EXIT_FAILURE);
            }

            double start = omp_get_wtime();

            int i;
            #pragma omp parallel for default(none) shared(A, B, C, N) private(i) schedule(static)
            for (i=0; i<N; i++){
                for (int j=0; j<N; j++){
                    C[i*N + j] = 0.0;
                    for (int k=0; k<N;k++){
                        C[i*N + j] += A[i*N + k] * B[k*N + j];
                    }
                }
            }


            double calc_end = omp_get_wtime();

            double checksum = 0.0, sumC = 0.0, maxC = -99999;
            #pragma omp parallel for default(none) shared(C, N, checksum, sumC, maxC) schedule(static)
            for (int i=0; i<N; i++){
                for (int j=0;j<N; j++){
                    #pragma omp critical
                    {
                        double value = C[i*N + j];
                        sumC += value;
                        if (maxC < value)
                            maxC = value;
                        checksum += (long long)(C[i*N + j]*1000.0)%100000;
                    }
                }
            }

            double end = omp_get_wtime();

            printf("Calculation time, Total Time, Checksum, SumC, MaxC\n");
            printf("%10lf %10lf %10lf %10lf %10lf\n", calc_end-start, end-start, checksum, sumC, maxC);

            // printf("Calculation time %f seconds\n", calc_end - start);
            // printf("Checksum calculation time %f seconds\n", end - calc_end);
            // printf("Checksum: %lf\n", checksum);
            // printf("Sum C: %lf\n", sumC);
            // printf("Max C: %lf\n", maxC);

            free(A);
            free(B);
            free(C);
            break;
        }
        case TASKS: {
            double start, calc_end, end;
            double maxC = -99999.0, sumC = 0.0, checksum = 0.0;

            double *C = malloc(sizeof(double) * N * N);
            if (C == NULL) {
                fprintf(stderr, "Error: Failed to allocate memory\n");
                exit(EXIT_FAILURE);
            }

            #pragma omp parallel default(none) shared(start, calc_end, end, A, B, C, N, sumC, maxC, checksum)
            {

                #pragma omp single
                {
                    printf("%ld CHECKSUM ATOMIC\n", N);
                    start = omp_get_wtime();
                    // calculates A*B = C
                    for (int i=0; i<N; i++){
                        #pragma omp task
                        {
                            for (int j=0; j<N; j++){
                                C[i*N + j] = 0.0;
                                for (int k=0; k<N;k++){
                                    C[i*N + j] += A[i*N + k] * B[k*N + j];
                                }
                            }
                        }
                    }

                    // calculates the checksum, sumC and maxC
                    // this task will only start when the multiplication step is complete
                    #pragma omp taskwait
                    calc_end = omp_get_wtime();

                    for (int i=0; i<N; i++){
                        #pragma omp task
                        {
                            for (int j=0;j<N; j++){
                                double value = C[i*N + j];

                                #pragma omp atomic
                                sumC += value;

                                #pragma omp atomic
                                checksum += (long long)(C[i*N + j]*1000.0)%100000;

                                #pragma omp critical
                                {
                                    if (maxC < value)
                                        maxC = value;
                                }
                            }
                        }
                    }

                    #pragma omp taskwait
                    end = omp_get_wtime();

                    printf("Calculation time, Total Time, Checksum, SumC, MaxC\n");
                    printf("%10lf %10lf %10lf %10lf %10lf\n", calc_end-start, end-start, checksum, sumC, maxC);

                    // printf("Calculation time %f seconds\n", calc_end - start);
                    // printf("Checksum calculation time %f seconds\n", end - calc_end);
                    // printf("Checksum: %lf\n", checksum);
                    // printf("Sum C: %lf\n", sumC);
                    // printf("Max C: %lf\n", maxC);
                }
            }


            break;
        }
        case SIMD_SINGLE_THREADED: {
            printf("%ld SIMD Single threaded\n", N);
            omp_set_num_threads(1);

            double start, calc_end, end;
            double maxC = -99999.0, sumC = 0.0, checksum = 0.0;

            double *C = malloc(sizeof(double) * N * N);
            if (C == NULL) {
                fprintf(stderr, "Error: Failed to allocate memory\n");
                exit(EXIT_FAILURE);
            }

            start = omp_get_wtime();

            #pragma omp parallel for default(none) shared(A, B, C, N) schedule(static)
            for (int i=0; i<N; i++){
                {
                    for (int j=0; j<N; j++){
                        double sum = 0.0;
                        #pragma omp simd reduction(+:sum)
                        for (int k=0; k<N;k++){
                            sum += A[i*N + k] * B[k*N + j];
                        }
                        C[i*N + j] = sum;
                    }
                }
            }

            calc_end = omp_get_wtime();

            for (int i=0; i<N; i++){
                for (int j=0;j<N; j++){
                    double value = C[i*N + j];
                    sumC += value;

                    if (maxC < value)
                        maxC = value;

                    checksum += (long long)(C[i*N + j]*1000.0)%100000;
                }
            }
            end = omp_get_wtime();

            printf("Calculation time, Total Time, Checksum, SumC, MaxC\n");
            printf("%10lf %10lf %10lf %10lf %10lf\n", calc_end-start, end-start, checksum, sumC, maxC);

            // printf("Calculation time %f seconds\n", calc_end - start);
            // printf("Checksum calculation time %f seconds\n", end - calc_end);
            // printf("Checksum: %lf\n", checksum);
            // printf("Sum C: %lf\n", sumC);
            // printf("Max C: %lf\n", maxC);
            break;
        }
        case SIMD_MULTI_THREADED: {
            printf("%ld SIMD Multi-threaded\n", N);

            double start, calc_end, end;
            double maxC = -99999.0, sumC = 0.0, checksum = 0.0;

            double *C = malloc(sizeof(double) * N * N);
            if (C == NULL) {
                fprintf(stderr, "Error: Failed to allocate memory\n");
                exit(EXIT_FAILURE);
            }

            start = omp_get_wtime();

            #pragma omp parallel for default(none) shared(A, B, C, N) schedule(static)
            for (int i=0; i<N; i++){
                {
                    for (int j=0; j<N; j++){
                        double sum = 0.0;
                        #pragma omp simd reduction(+:sum)
                        for (int k=0; k<N;k++){
                            sum += A[i*N + k] * B[k*N + j];
                        }
                        C[i*N + j] = sum;
                    }
                }
            }

            calc_end = omp_get_wtime();

            // #pragma omp parallel for default(none) shared(C, N, checksum, sumC, maxC) schedule(static)
            for (int i=0; i<N; i++){
                for (int j=0;j<N; j++){
                    // #pragma omp critical
                    // {
                    double value = C[i*N + j];
                    sumC += value;

                    if (maxC < value)
                        maxC = value;

                    checksum += (long long)(C[i*N + j]*1000.0)%100000;
                    // }
                }
            }
            end = omp_get_wtime();

            printf("Calculation time, Total Time, Checksum, SumC, MaxC\n");
            printf("%10lf %10lf %10lf %10lf %10lf\n", calc_end-start, end-start, checksum, sumC, maxC);

            // printf("Calculation time %f seconds\n", calc_end - start);
            // printf("Checksum calculation time %f seconds\n", end - calc_end);
            // printf("Checksum: %lf\n", checksum);
            // printf("Sum C: %lf\n", sumC);
            // printf("Max C: %lf\n", maxC);
            break;
        }
        default: {
            fprintf(stderr, "Error: Mode <%s> is undefined\n", argv[2]);
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

void populate_matrixes(const size_t N, double *A, double *B) {
    if (A == NULL || B == NULL) {
        fprintf(stderr, "Error: failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = sin(i) * cos(j) + sqrt(i + j + 1);
            B[i * N + j] = cos(i) * sin(j) + sqrt(i + j + 2);
        }
    }
}

MODE_T get_type(char *arg) {
    if (strcmp(arg, "0") == 0)
        return SERIAL;
    else if (strcmp(arg, "1") == 0)
        return WORK_SHARING;
    else if (strcmp(arg, "2") == 0)
        return COLLAPSE;
    else if (strcmp(arg, "3A") == 0)
        return CHECKSUM_ATOMIC;
    else if (strcmp(arg, "3B") == 0)
        return CHECKSUM_CRITICAL;
    else if (strcmp(arg, "4") == 0)
        return TASKS;
    else if (strcmp(arg, "5") == 0)
        return SIMD_SINGLE_THREADED;
    else if (strcmp(arg, "6") == 0)
        return SIMD_MULTI_THREADED;
    else
        return -1;
}
