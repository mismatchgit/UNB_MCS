#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <omp.h>

//print for us the error message
static void die(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

//convert the input to long
static long parse_long(const char *s, const char *name) {
    errno = 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') {
        fprintf(stderr, "Error: invalid %s '%s'\n", name, s);
        exit(EXIT_FAILURE);
    }
    return v;
}

//format the 2D array to 1D row-column major
static size_t idx(size_t i, size_t j, size_t N) {
    return i * N + j;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "structure is : %s N mode\n", argv[0]);
        fprintf(stderr, "  N: matrix dimension (e.g: 512, 1024, 2048, …)\n");
        fprintf(stderr, "  mode: (0 to 6) \n");
        return EXIT_FAILURE;
    }

    long N_in = parse_long(argv[1], "N");
    long mode_in = parse_long(argv[2], "mode");

    if (N_in <= 0) die("N must be > 0");
    if (mode_in < 0 || mode_in > 6) die("mode must be between 0 and 6");

    const size_t N = (size_t)N_in;
    const int mode = (int)mode_in;

    // Overflow-safe element count check: N*N must fit in size_t
    if (N != 0 && N > (SIZE_MAX / N)) die("N is too large (N*N overflows)");
    const size_t elems = N * N;

    // Allocate contiguous matrices
    double *A = (double*)malloc(elems * sizeof(double));
    double *B = (double*)malloc(elems * sizeof(double));
    double *C = (double*)malloc(elems * sizeof(double));
    if (!A || !B || !C) die("malloc failed (not enough memory)");

    // Deterministic initialization with the same formula
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            A[idx(i,j,N)] = sin((double)i) * cos((double)j) + sqrt((double)(i + j + 1));
            B[idx(i,j,N)] = cos((double)i) * sin((double)j) + sqrt((double)(i + j + 2));
            C[idx(i,j,N)] = 0.0;
        }
    }

    double serial_multiplication, serial_multiplication_end, start_s,end_s = 0.0;

    //start with mode 0 (serial)
    if (mode ==0){
        int threads_used = 1; //since it's serial
        //start serial time
        start_s = omp_get_wtime();
        //implement the multiplication and adding it to c
        serial_multiplication= omp_get_wtime();
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double sum = 0.0;
                for (size_t k = 0; k < N; k++) {
                    sum += A[idx(i,k,N)] * B[idx(k,j,N)];
                }
                C[idx(i,j,N)] = sum;
            }
        }
        serial_multiplication_end = omp_get_wtime();

        double final_m= serial_multiplication_end - serial_multiplication;
        
        //calcylate the sum, max and the checksum formula
        double sumC = 0.0;
        double maxC = -INFINITY;
        long long checksum = 0;
        double serial_sum_max_check, serial_sum_max_check_end = 0.0;
        serial_sum_max_check = omp_get_wtime();
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double v = C[idx(i,j,N)];
                sumC += v;
                if (v > maxC) maxC = v;

                // cast it from float to integers
                long long term = ((long long)(v * 1000.0)) % 100000;
                checksum += term;
            }
        }

        serial_sum_max_check_end = omp_get_wtime();
        end_s = omp_get_wtime();
        //time to run the multiplocation,sum andb checksum
        double final_sum_max= serial_sum_max_check_end-serial_sum_max_check;
        //whole time to run serial
        double final_s=end_s - start_s ;

        printf("N=%zu mode=%d serial_thread=%d\n", N, mode, threads_used);
        printf("Matrix Multiplication and C serial time =%.6f\n", final_m);
        printf("SUM, Max, checkSum time =%.6f\n", final_sum_max);
        printf("Total serial time=%.6f\n", final_s);
        printf("sumC=%.6f maxC=%.6f checksum=%lld\n", sumC, maxC, checksum);

    }else if (mode ==1)
    {   
        double start_mode1, end_mode1 = 0.0;
        start_mode1 = omp_get_wtime();
        //change the static to sechedule(dynamic, 5) and re-run & (can't use private  as am using parallel for)
        #pragma omp parallel for schedule(static) default(none) shared(A,B,C,N)
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double sum = 0.0;
                for (size_t k = 0; k < N; k++) {
                    sum += A[idx(i,k,N)] * B[idx(k,j,N)];
                }
                C[idx(i,j,N)] = sum;
            }
        }
        end_mode1 = omp_get_wtime();
        double final = end_mode1-start_mode1;
        printf("N=%zu mode=%d omp_max_threads=%d\n", N, mode, omp_get_max_threads());
        printf("Matrix Multiplication C using (Parallel for schedule) time =%.6f\n", final);

        //calcylate the sum, max and the checksum formula as a way to check correctness of C
        double sumC = 0.0;
        double maxC = -INFINITY;
        long long checksum = 0;
        double serial_sum_max_check, serial_sum_max_check_end = 0.0;
        serial_sum_max_check = omp_get_wtime();
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double v = C[idx(i,j,N)];
                sumC += v;
                if (v > maxC) maxC = v;

                // cast it from float to integers
                long long term = ((long long)(v * 1000.0)) % 100000;
                checksum += term;
            }
        }

        printf("sumC=%.6f maxC=%.6f checksum=%lld\n", sumC, maxC, checksum);
    }else if (mode == 2)
    {
        double start_mode2, end_mode2 = 0.0;
        start_mode2 = omp_get_wtime();
        //using collapse 2 for matrix multiplication
        //For experiment you may still change static to dynamic
        #pragma omp parallel for collapse(2) schedule(static) default(none) shared(A, B, C, N)
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double sum = 0.0;  
                for (size_t k = 0; k < N; k++) {
                    sum += A[idx(i,k,N)] * B[idx(k,j,N)];
                }
                C[idx(i,j,N)] = sum;
            }
        }

        end_mode2 = omp_get_wtime();
        double final_mode2 = end_mode2 - start_mode2;
        printf("N=%zu mode=%d omp_max_threads=%d\n", N, mode, omp_get_max_threads());
        printf("Matrix Multiplication C using (Parallel for collapse schedule) time =%.6f\n", final_mode2);
        //calcylate the sum, max and the checksum formula as a way to check correctness of C
        double sumC = 0.0;
        double maxC = -INFINITY;
        long long checksum = 0;
        double serial_sum_max_check, serial_sum_max_check_end = 0.0;
        serial_sum_max_check = omp_get_wtime();
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double v = C[idx(i,j,N)];
                sumC += v;
                if (v > maxC) maxC = v;

                // cast it from float to integers
                long long term = ((long long)(v * 1000.0)) % 100000;
                checksum += term;
            }
        }
        printf("sumC=%.6f maxC=%.6f checksum=%lld\n", sumC, maxC, checksum);

    }else if (mode == 3){
        double start_mode3, end_mode3 = 0.0;
        start_mode3 = omp_get_wtime();

        //Matrix multiplication to find c (parallel for collapse(2))
        #pragma omp parallel for schedule(static) default(none) shared(A, B, C, N)
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double sum = 0.0;
                for (size_t k = 0; k < N; k++) {
                    sum += A[idx(i,k,N)] * B[idx(k,j,N)];
                }
                C[idx(i,j,N)] = sum;
            }
        }

        //3A. parallize the calculation of sum, max ( use reduction) and checksum atomic 
        double sumC_atomic = 0.0;
        double maxC_atomic = -INFINITY;
        long long checksum_atomic = 0;

        //time for atomic
        double atomic_start = omp_get_wtime();
        #pragma omp parallel for collapse(2) schedule(static) default(none) shared(C, N, checksum_atomic) reduction(+:sumC_atomic) reduction(max:maxC_atomic)
                for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double v = C[idx(i,j,N)];
                sumC_atomic += v;
                if (v > maxC_atomic) maxC_atomic = v;
                long long term = ((long long)(v * 1000.0)) % 100000;
                #pragma omp atomic
                checksum_atomic += term;
            }
        }
        double atomic_end = omp_get_wtime();
        double atomic_time = atomic_end - atomic_start;

        //3B Implement the same with critical
        double sumC_critical = 0.0;
        double maxC_critical = -INFINITY;
        long long checksum_critical = 0;
        //time for critical
        double critical_start = omp_get_wtime();
        #pragma omp parallel for collapse(2) schedule(static) default(none) shared(C, N, checksum_critical) reduction(+:sumC_critical) reduction(max:maxC_critical)
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double v = C[idx(i,j,N)];
                sumC_critical += v;
                if (v > maxC_critical) maxC_critical = v;

                long long term = ((long long)(v * 1000.0)) % 100000;
                #pragma omp critical
                {
                    checksum_critical += term;
                }
            }
        }
        double critical_end = omp_get_wtime();
        double critical_time = critical_end - critical_start;

        printf("N=%zu mode=%d omp_max_threads=%d\n", N, mode, omp_get_max_threads());
        printf("Atomic loop time 3A =%.6f sumC_atomic=%.6f maxC_atomic=%.6f checksum_atomic=%lld\n",
            atomic_time, sumC_atomic, maxC_atomic, checksum_atomic);
        printf("Critical loop time 3B =%.6f sumC_critical=%.6f maxC_critical=%.6f checksum_critical=%lld\n",
            critical_time, sumC_critical, maxC_critical, checksum_critical);

    }
    else if (mode == 4)
    { 
        //tasks per row block (these can be modified to 32,...) 
        const size_t block_rows = 16;
        double task_mat_mul_start = omp_get_wtime();
        #pragma omp parallel default(none) shared(A, B, C, N, block_rows)
        {
            #pragma omp single
            {
                #pragma omp taskgroup
                {
                    for (size_t i_start = 0; i_start < N; i_start += block_rows){
                        size_t i_end = i_start + block_rows;
                        //not overpass N
                        if (i_end > N) i_end = N;
                        //give each task it's chunk or block
                        #pragma omp task firstprivate(i_start, i_end) shared(A, B, C, N)
                        {
                            for (size_t i = i_start; i < i_end; i++){
                                for (size_t j = 0; j < N; j++){
                                    double sum = 0.0;
                                    for (size_t k = 0; k < N; k++){
                                        sum += A[idx(i,k,N)] * B[idx(k,j,N)];
                                    }
                                    C[idx(i,j,N)] = sum;
                                }
                            }
                        }

                    }
                }
            }
        }
        double task_mat_mul_end = omp_get_wtime();
        double task_mat_mul_final = task_mat_mul_end - task_mat_mul_start; 

        //Check if the value of c is right calculating sum, max, checksum
        double sumC_atomic = 0.0;
        double maxC_atomic = -INFINITY;
        long long checksum_atomic = 0;

        #pragma omp parallel for collapse(2) schedule(static) default(none) shared(C, N, checksum_atomic) reduction(+:sumC_atomic) reduction(max:maxC_atomic)
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double v = C[idx(i,j,N)];
                sumC_atomic += v;
                if (v > maxC_atomic) maxC_atomic = v;
                long long term = ((long long)(v * 1000.0)) % 100000;
                #pragma omp atomic
                checksum_atomic += term;
            }
        }
        printf("N=%zu mode=%d omp_max_threads=%d\n", N, mode, omp_get_max_threads());
        printf("Matrix Multiplication C using tasks (mode 4) time =%.6f\n", task_mat_mul_final);
        printf("sumC=%.6f maxC=%.6f checksum=%lld\n", sumC_atomic, maxC_atomic, checksum_atomic);

    }else if(mode==5){
        int threads_used = 1;
        double start_mode5, end_mode5 = 0.0;
        start_mode5 = omp_get_wtime();
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double sum = 0.0;
                // SIMD-ize the dot product over k
                #pragma omp simd reduction(+:sum)
                for (size_t k = 0; k < N; k++) {
                    sum += A[idx(i,k,N)] * B[idx(k,j,N)];
                }

                C[idx(i,j,N)] = sum;
            }
        }

        end_mode5= omp_get_wtime();
        double final_mat_mul_time= end_mode5 - start_mode5;

        //calculate sum,max and checksum for verification
        double sumC_atomic = 0.0;
        double maxC_atomic = -INFINITY;
        long long checksum_atomic = 0;

        #pragma omp parallel for collapse(2) schedule(static) default(none) shared(C, N, checksum_atomic) reduction(+:sumC_atomic) reduction(max:maxC_atomic)
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double v = C[idx(i,j,N)];
                sumC_atomic += v;
                if (v > maxC_atomic) maxC_atomic = v;
                long long term = ((long long)(v * 1000.0)) % 100000;
                #pragma omp atomic
                checksum_atomic += term;
            }
        }

        printf("N=%zu mode=%d omp_max_threads=%d\n", N, mode, threads_used);
        printf("Matrix Multiplication C using tasks (mode 5) time =%.6f\n", final_mat_mul_time);
        printf("sumC=%.6f maxC=%.6f checksum=%lld\n", sumC_atomic, maxC_atomic, checksum_atomic);

    }else if (mode == 6){
        double start_mode6, end_mode6 = 0.0;
        start_mode6 = omp_get_wtime();

        //parallize it using omp parallel for 
        
        #pragma omp parallel for schedule(static) default(none) shared(A, B, C, N)
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double sum = 0.0;
                #pragma omp simd reduction(+:sum)
                for (size_t k = 0; k < N; k++) {
                    sum += A[idx(i,k,N)] * B[idx(k,j,N)];
                }
                C[idx(i,j,N)] = sum;
            }
        }
        end_mode6 = omp_get_wtime();
        double final_mat_mul_time= end_mode6 - start_mode6;

        //calculate sum,max and checksum for verification
        double sumC_atomic = 0.0;
        double maxC_atomic = -INFINITY;
        long long checksum_atomic = 0;

        #pragma omp parallel for collapse(2) schedule(static) default(none) shared(C, N, checksum_atomic) reduction(+:sumC_atomic) reduction(max:maxC_atomic)
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double v = C[idx(i,j,N)];
                sumC_atomic += v;
                if (v > maxC_atomic) maxC_atomic = v;
                long long term = ((long long)(v * 1000.0)) % 100000;
                #pragma omp atomic
                checksum_atomic += term;
            }
        }

        printf("N=%zu mode=%d omp_max_threads=%d\n", N, mode, omp_get_max_threads());
        printf("Matrix Multiplication C using tasks (mode 6) time =%.6f\n", final_mat_mul_time);
        printf("sumC=%.6f maxC=%.6f checksum=%lld\n", sumC_atomic, maxC_atomic, checksum_atomic);

    }
      

    free(A);
    free(B);
    free(C);
    return EXIT_SUCCESS;
}
