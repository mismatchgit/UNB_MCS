
//TODO1: Step 0: Clean file structure
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
//4.1. Step 1C--> allocation of memory spaces of matrix A, B, C
static double* alloc_matrix(int N) {
    double *m = (double*)malloc((size_t)N * (size_t)N * sizeof(double));
    //condition to check if malloc fails and returns NULL
    if (m == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    return m;
}
int main(int argc, char **argv){
    // printf("\n The number of arguments in argc = %d",argc);
    //1. the condition below checks whether this argc has < 3 values 
    //meaning size of matrix and number of threads are nnot giverm
    if (argc < 3) {
    printf("\n Usage: ./a1 N mode\n");
    return 1;
}
    
    // if(argc==3){
    //     for(int i = 0;i<argc;i++){
    //     printf("%s\n",argv[i]);
    // }

    // }
    //2. Step 1A: Extracting argument values of argv, and converting them to integer
    int N = atoi(argv[1]);
    int mode = atoi(argv[2]);
    // printf("N = %d and mode = %d\n",N,mode);
    //3. Step 1B: Basic safety Check for N
    if (N<1){
        printf("Error! N should be greater than 0\n");
        return 1;
    }
    //4. Step 1C: Allocate Matrices
    double *A = alloc_matrix(N);
    double *B = alloc_matrix(N);
    double *C = alloc_matrix(N);
    //total start time
   double total_start = omp_get_wtime();
    //5. Step 2: initialize A, B, C
    for(int i =0; i<N;i++){
        for(int j =0; j<N; j++){
            A[i*N + j] = sin((double)i) * cos((double)j) + sqrt((double)(i + j + 1));
            B[i*N + j] = cos((double)i) * sin((double)j) + sqrt((double)(i + j + 2));
            C[i*N+j]= 0.0;        
        }
    }
    //7. Step 3B:Adding Kernel timing
    double kernel_start_time = omp_get_wtime();
    
    switch (mode){
        case 0:{
        //6. Step 3A: multiplication loop for mode 0
            for(int i =0; i<N; i++){
                for(int j =0; j<N; j++){
                    double sum = 0.0;
                    for(int k =0;k<N;k++)
                    sum += A[i*N+k] *B[k*N+j];
                    C[i*N+j] = sum;

                }
            }
        }
        break;
        case 1:{
        //10. Step 4: Parallel scheduling
        #pragma omp parallel for schedule(static)
        for(int i =0; i<N; i++){
                for(int j =0; j<N; j++){
                    double sum = 0.0;
                    for(int k =0;k<N;k++)
                    sum += A[i*N+k] *B[k*N+j];
                    C[i*N+j] = sum;

                }
            }
        }
        break;
        case 2:{
        //11. Step 5: Mode2: collapse(2)
        #pragma omp parallel for collapse(2) schedule(static)
        for(int i =0; i<N; i++){
                for(int j =0; j<N; j++){
                    double sum = 0.0;
                    for(int k =0;k<N;k++)
                    sum += A[i*N+k] *B[k*N+j];
                    C[i*N+j] = sum;

                }
            }
        }
        break;
        //12. Step 6: Mode3A:atomic
        case 3:{
        
            #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                double sum = 0.0;
                for (int k = 0; k < N; k++) {
                    sum += A[i*N + k] * B[k*N + j];
                }
                C[i*N + j] = sum;
            }
        }
        break;
    }
        case 4:{
        //13. critical check sum
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                double sum = 0.0;
                for (int k = 0; k < N; k++) {
                    sum += A[i*N + k] * B[k*N + j];
                }
                C[i*N + j] = sum;
            }
        }
    }
        break;
        case 5:{
        //14. Step 7. Tasks
        int block_size = 16;
        #pragma omp parallel
        {
            
            #pragma omp single
            {
                
                    
                    for(int i = 0; i<N; i +=block_size ){
                        int i_start = i;
                        int i_end = (i+block_size<N) ? (i+block_size):N;
                        #pragma omp task firstprivate(i_start,i_end)
                        {
                            for(int ii =i_start; ii<i_end; ii++){
                                for(int j =0; j<N; j++){
                                    double sum = 0.0;
                                    for(int k =0;k<N;k++)
                                        sum += A[ii*N+k] *B[k*N+j];
                                    C[ii*N+j] = sum;

                                }
                            }
                       
                    }
                
                }
                #pragma omp taskwait
            }
        }
    }
            break;
        case 6:{
            //15. Step 8 SIMD
            #pragma omp parallel for schedule(static)
             for(int i =0; i<N; i++){
                for(int j =0; j<N; j++){
                    double sum = 0.0;
                    #pragma omp simd reduction(+:sum)
                    
                    for(int k =0;k<N;k++)
                    sum += A[i*N+k] *B[k*N+j];
                    C[i*N+j] = sum;

                
            }
            }

        }
        break;

    }

    //7.1: Step 3B: Adding Kernel Timing --> end time
   double kernel_end_time = omp_get_wtime();
   double total_kernel_time = kernel_end_time - kernel_start_time;
//    printf("Kernel Start Time = %f\n",kernel_start_time);
//    printf("Kernel End Time = %f\n",kernel_end_time);
    
    //8. Step 3C: Sanity Checks
    printf("c[0] =%f\n",C[0]);
    printf("c[last] =%f\n", C[N*N-1]);
    //9. Step 3D: Compute Analytics(serial)
   
double sumC = 0.0;
double maxC = C[0];

for (int idx = 0; idx < N*N; idx++) {
    double x = C[idx];
    sumC += x;
    if (x > maxC) maxC = x;
}

// Analytics: checksum (mode-dependent)

long long checksum = 0;
double t_ck_start = omp_get_wtime();

if (mode == 3) {
    // Mode 3: checksum using atomic
    #pragma omp parallel for
    for (int idx = 0; idx < N*N; idx++) {
        long long contrib = (long long)(C[idx] * 1000.0) % 100000;
        #pragma omp atomic
        checksum += contrib;
    }
}
else if (mode == 4) {
    // Mode 4: checksum using critical
    #pragma omp parallel for
    for (int idx = 0; idx < N*N; idx++) {
        long long contrib = (long long)(C[idx] * 1000.0) % 100000;
        #pragma omp critical
        {
            checksum += contrib;
        }
    }
}
else {
    // Modes 0/1/2: checksum serial (baseline)
    for (int idx = 0; idx < N*N; idx++) {
        checksum += (long long)(C[idx] * 1000.0) % 100000;
    }
}

double t_ck_end = omp_get_wtime();
double checksum_time = t_ck_end - t_ck_start;


double total_end = omp_get_wtime();
double total_time = total_end - total_start;

printf("Kernel Time = %f\n", total_kernel_time);
// printf("Total Time = %f\n", total_time);
printf("MaxC = %f\n", maxC);
printf("sumC = %f\n", sumC);
printf("checksum = %lld\n", checksum);
// printf("checksum_time = %f\n", checksum_time);

    return 0;
}
