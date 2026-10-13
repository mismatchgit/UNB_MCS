#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 4000

double* alloc_matrix(int n) {
    double *m = (double*) malloc((size_t)n * (size_t)n * sizeof(double));
    if (!m) {
        printf("Allocation failed!\n");
        exit(1);
    }
    return m;
}

void fill_matrix(double *m, int n, double val) {
    for (int i = 0; i < n*n; i++) m[i] = val;
}

int main(int argc, char **argv) {

    // Optional: run like ./matrix_lab 6
    if (argc >= 2) {
        int t = atoi(argv[1]);
        if (t > 0) omp_set_num_threads(t);
    }

    double *A = alloc_matrix(N);
    double *B = alloc_matrix(N);
    double *C = alloc_matrix(N);

    fill_matrix(A, N, 1.0);
    fill_matrix(B, N, 1.0);
    fill_matrix(C, N, 0.0);

    // =====================================================
    // LEVEL 1: SERIAL BASELINE
    // =====================================================
//to print A
// int subset = 3; 
// printf("Top-left %dx%d of Matrix A:\n", subset, subset);

// for (int i = 0; i < N; i++) {
//     for (int j = 0; j < N; j++) {
//         // Accessing A as a 1D array using (row * N + col)
//         printf("%6.1f ", A[i * N + j]);
//     }
//     printf("\n");
// }
    // TODO: Implement Level 1 here
    printf("\n--------Level 1 ----------\n");
double start_time = omp_get_wtime();    

for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
        // C[i][j] = A[i][j] + B[i][j]
        C[i * N + j] = A[i * N + j] + B[i * N + j];
    }
}

// Leve1
double end_time = omp_get_wtime();
double total_time = end_time - start_time;

// Correctness check
printf("Level 1 start time: %f seconds\n", start_time);
printf("Level 1 end time: %f seconds\n", end_time);
printf("Level 1 time: %f seconds\n", total_time);
printf("C[N-1][N-1] = %f\n", C[(N-1) * N + (N-1)]); 
//to print C

// int subset = 3; 
// printf("Top-left %dx%d of Matrix C:\n", subset, subset);

// for (int i = 0; i < N; i++) {
//     for (int j = 0; j < N; j++) {
//         // Accessing A as a 1D array using (row * N + col)
//         printf("%6.1f ", C[i * N + j]);
//     }
//     printf("\n");
// }
    // TODO: Implement Level 1 here
  

for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
        // C[i][j] = A[i][j] + B[i][j]
        C[i * N + j] = A[i * N + j] + B[i * N + j];
    }
}
    // =====================================================
    // LEVEL 2: MANUAL BLOCK DECOMPOSITION
    // =====================================================
    
    // TODO: Implement Level 2A here
    printf("\n--------Level 2A ----------");
#pragma omp parallel
{
     
    int id = omp_get_thread_num();
    int nthr = omp_get_num_threads();
    printf("The Thread id is %d of %d\n",id,nthr);
    
    int chunk = N/ nthr;
    int start = id * chunk;
    int end = start + chunk;
    for (int i = start; i < end; i++) {
    for (int j = start; j < end; j++) {
        // C[i][j] = A[i][j] + B[i][j]
        C[i * N + j] = A[i * N + j] + B[i * N + j];
    }
}
printf("C = %f\n", C[(N-1) * N + (N-1)]); 
printf("Chunk:%d and Start:%d and End:%d \n",chunk,start,end);

}
//------------------------------------------------------------------------------
    // TODO: Implement Level 2C here
    printf("\n--------Level 2C ----------");
   double start_time_L2 = omp_get_wtime(); 
   #pragma omp parallel
{
        int Thread = omp_get_max_threads();
int recal_chunk = N/Thread;

    int ThreadID = omp_get_thread_num();
    int start = ThreadID * recal_chunk;
    int end = (ThreadID == Thread - 1)?N:(start+recal_chunk);
    for (int i = start; i < end; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = A[i * N + j] + B[i * N + j];
        }
    }
    
printf("Chunk:%d and Start:%d and End:%d \n",recal_chunk,start,end);
    }
    double end_time_L2 = omp_get_wtime();
double total_time_l2 = end_time_L2 - start_time_L2;
printf("Level 2 time: %f seconds\n", total_time_l2);


    // =====================================================
    // LEVEL 3: CYCLIC DECOMPOSITION
    // =====================================================

    // TODO: Implement Level 3 here
printf("\n--------Level 3----------");
    double start_time_L3 = omp_get_wtime();
    #pragma omp parallel
    {
        int thrdID = omp_get_thread_num();
        int Thread = omp_get_num_threads();
        //Round robin method used here
        for(int i = thrdID; i<N ; i+=Thread){
            for(int j = 0;j<N; j++){
               C[i * N + j] = A[i * N + j] + B[i * N + j]; 
            }
        }
    }
    printf("Level 3 time: %f\n", omp_get_wtime() - start_time_L3);

    // =====================================================
    // LEVEL 4: DEPENDENCY TRAP
    // =====================================================

    // TODO: Implement Level 4A here
fill_matrix(C, N, 1.0); // Reinitialize C to 1.0
for (int i = 1; i < N; i++) {
    for (int j = 0; j < N; j++) {
        C[i * N + j] = C[i * N + j] + C[(i - 1) * N + j];
    }
}
printf("Level 4A (Serial) Checkpoint C[1000][0]: %f\n", C[1000 * N + 0]);
//-------------------------------------------------------------------
printf("\n--------Level 4B ----------");
    // TODO: Implement Level 4B here
    fill_matrix(C, N, 1.0); // Reset C again
int T4 = 4; // Lab asks for 4 threads
omp_set_num_threads(T4);
int chunk4 = N / T4;

#pragma omp parallel
{
    int tid = omp_get_thread_num();
    int start = (tid == 0) ? 1 : tid * chunk4; 
    int end = (tid == T4 - 1) ? N : (tid + 1) * chunk4;

    for (int i = start; i < end; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = C[i * N + j] + C[(i - 1) * N + j];
        }
    }
}
printf("Level 4B (Parallel) Checkpoint C[1000][0]: %f\n", C[1000 * N + 0]);
    // =====================================================
    // LEVEL 5: BARRIER SYNCHRONIZATION
    // =====================================================
printf("\n =============================================");
printf("\n LEVEL 5: BARRIER SYNCHRONIZATION");
printf("\n =============================================");
    // TODO: Implement Level 5A here
    fill_matrix(C, N, 0.0); 
int Thread5 = 4;
omp_set_num_threads(Thread5);
int chunk5 = N / Thread5;

#pragma omp parallel
{
    int thrdid = omp_get_thread_num();
    int start = thrdid * chunk5;
    int end = (thrdid == Thread5 - 1) ? N : (start + chunk5);

    // Phase 1: Calculation
    for (int i = start; i < end; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = A[i * N + j] + B[i * N + j];
        }
    }
    printf("Thread %d finished calculation\n", thrdid);

    //--------------------------------------------------------
    // TODO: Implement Level 5B here
    printf("\n--------Level 5B ----------");
#pragma omp barrier 

    // Phase 2: Verification
    printf("Thread %d starting verification\n", thrdid);
}
    free(A);
    free(B);
    free(C);
    return 0;
}