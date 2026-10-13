#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 4000

double* alloc_matrix(int n) 
{
    double *m = (double*) malloc((size_t)n * (size_t)n * sizeof(double));
    if (!m) 
    {
        printf("Allocation failed!\n");
        exit(1);
    }
    return m;
}

void fill_matrix(double *m, int n, double val) 
{
    for (int i = 0; i < n*n; i++) m[i] = val;
}

int main(int argc, char **argv) 
{

    int T;

    // Optional: run like ./matrix_lab 6
    if (argc >= 2) 
    {
        int t = atoi(argv[1]);
        if (t > 0) omp_set_num_threads(t);
        T = t;
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

    printf("\n\nLEVEL 1 CALCULATIONS: \n\n");

    double start1 = omp_get_wtime();

    for(int j=0; j<N; j++)
    {
        for(int i=0; i<N; i++)
        {
            C[i*N+j] = A[i*N+j] + B[i*N+j];
        }
    }

    double end1 = omp_get_wtime();

    double totalTime1 = end1 - start1;

    printf("Serial Time Total: %f\n", totalTime1);
    
    printf("Correctness Check: %f\n",C[(N-1)*N + (N-1)]);

    // =====================================================
    // LEVEL 2: MANUAL BLOCK DECOMPOSITION
    // =====================================================

    printf("\n\nLEVEL 2A CALCULATIONS: \n\n");

    double start2a = omp_get_wtime();

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int chunk = N / T;
        int start_row = id * chunk;
        int end_row = start_row + chunk;
        for (int j = 0; j < N; j++)
        {
            for (int i = start_row; i < end_row; i++)
            {
                C[i*N+j] = A[i*N+j] + B[i*N+j];
            }
        }
    }

    double end2a = omp_get_wtime();

    double totalTime2a = end2a - start2a;

    printf("Serial Time Total: %f\n", totalTime2a);
    
    printf("Correctness Check: %f\n",C[(N-1)*N + (N-1)]);

    //-----------------------------------------------------------

    printf("\n\nLEVEL 2C CALCULATIONS: \n\n");

    double start2c = omp_get_wtime();

    int remainder =  N % T;
    int chunk = (N-remainder)/T;

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int start_row = id * chunk;
        int end_row = start_row + chunk;

        if (id == T-1)
        {
            end_row += remainder;
        }

        for (int j = 0; j < N; j++)
        {
            for (int i = start_row; i < end_row; i++)
            {
                C[i*N+j] = A[i*N+j] + B[i*N+j];
            }
        }
    }

    double end2c = omp_get_wtime();

    double totalTime2c = end2c - start2c;

    printf("Serial Time Total: %f\n", totalTime2c);
    
    printf("Correctness Check: %f\n",C[(N-1) * N + (N-1)]);


    // =====================================================
    // LEVEL 3: CYCLIC DECOMPOSITION
    // =====================================================

    printf("\n\nLEVEL 3 CALCULATIONS: \n\n");

    double start3 = omp_get_wtime();

    remainder = N % T;

    chunk = (N-remainder)/T;

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int nthr = omp_get_num_threads();

        for (int j = 0; j < N; j++)
        {
            for (int i = id; i < N; i += nthr)
            {
                C[i*N+j] = A[i*N+j] + B[i*N+j];
            }
        }
    }

    double end3 = omp_get_wtime();

    double totalTime3 = end3 - start3;

    printf("Serial Time Total: %f\n", totalTime3);
    
    printf("Correctness Check: %f\n",C[(N-1) * N + (N-1)]);

    
    double speedup = totalTime1/totalTime3;

    printf("Speedup of Level 3 vs Level 1: %f", speedup);


    // =====================================================
    // LEVEL 4: DEPENDENCY TRAP
    // =====================================================
    
    printf("\n\nLEVEL 4A CALCULATIONS: \n\n");

    fill_matrix(C, N, 1.0); //reinitialize C with a known value

    for(int j=0; j<N; j++)
    {
        for(int i=0; i<N; i++)
        {
            C[i*N+j] = A[i*N+j] + B[i*N+j];
        }
    }
    
    printf("Correctness Check: %f\n",C[1000]);

    //------------------------------------------------------

    printf("\n\nLEVEL 4B CALCULATIONS: \n\n");

    fill_matrix(C, N, 1.0); //reinitialize C with a known value

    omp_set_num_threads(4); //set to 4 threads

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int chunk = N/T;
        int start_row = id * chunk;
        int end_row = start_row + chunk;

        for (int j = 0; j < N; j++)
        {
            for (int i = start_row; i < end_row; i++)
            {
                if(i>0)
                {
                    C[i*N+j] += C[(i-1)*N+j];
                }
            }
        }
    }

    printf("Correctness Check: %f\n",C[1000]);

    // =====================================================
    // LEVEL 5: BARRIER SYNCHRONIZATION
    // =====================================================

    printf("\n\nLEVEL 5A CALCULATIONS: \n\n");

    fill_matrix(C, N, 1.0); //reinitialize C with a known value

    omp_set_num_threads(4); //set to 4 threads

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int chunk = N/T;
        int start_row = id * chunk;
        int end_row = start_row + chunk;
        
        for (int j = 0; j < N; j++)
        {
            for (int i = start_row; i < end_row; i++)
            {
                C[i*N+j] = A[i*N+j] + B[i*N+j];
            }
        }
        printf("\nThread %d finished calculation\n", id);
        printf("\nThread %d starting verification\n", id);
    }

    //------------------------------------------------------

    printf("\n\nLEVEL 5B CALCULATIONS: \n\n");

    fill_matrix(C, N, 1.0); //reinitialize C with a known value

    omp_set_num_threads(4); //set to 4 threads

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int chunk = N/T;
        int start_row = id * chunk;
        int end_row = start_row + chunk;
        
        for (int j = 0; j < N; j++)
        {
            for (int i = start_row; i < end_row; i++)
            {
                C[i*N+j] = A[i*N+j] + B[i*N+j];
            }
        }
        printf("\nThread %d finished calculation\n", id);
        #pragma omp barrier
        printf("\nThread %d starting verification\n", id);
    }

    free(A);
    free(B);
    free(C);
    return 0;
}
