#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <omp.h>


bool checksumVerification(int[] C)
{
    int checksum = 0;
    for() //every value in the matrix
    {
        checksum += (long long)(C[i*N + j] * 1000.0) % 100000;
    }
    if (checksum) //some if statement
    {
        return true;
    }
    else return false;
}

void createAandBMatrices(int N)
{
    double A[] = double * malloc(sizeof(double) * N * N);
    double B[] = double * malloc(sizeof(double) * N * N);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            A[i * N + j] = sin(i) * cos(j) + sqrt(i + j + 1); //random number initialization
            B[i * N + j] = cos(i) * sin(j) + sqrt(i + j + 2); //use -lm when compiling
        }
    }

}

void serial()
{
    double startTotal = omp_get_wtime();
    //Compute AxB
    double startKernal = omp_get_wtime();


    double endKernal = omp_get_wtime();
    double timeKernal = endKernal - startKernal;
    printf("Mode 0: Serial. Kernal Processing time taken: %f\n", timeKernal);
    //Compute sumC

    //Compute maxC

    //Compute checksum

    double endTotal = omp_get_wtime();
    double timeTotal = end-start;
    printf("Mode 0: Serial. Total Processing time taken: %f\n", timeTotal);
}

void openMP()
{
    double start = omp_get_wtime();


    double end = omp_get_wtime();
    double time = end-start;
    printf("Mode 1: OpenMP. Processing time taken: %f\n", time);
}

void openMPCollapse()
{
    double start = omp_get_wtime();


    double end = omp_get_wtime();
    double time = end-start;
    printf("Mode 2: OpenMPCollapse. Processing time taken: %f\n", time);
}


void synchronizationComparison()
{
    double start = omp_get_wtime();


    double end = omp_get_wtime();
    double time = end-start;
    printf("Mode 3: SynchronizationComparison. Processing time taken: %f\n", time);
}

void taskBasedParallelism()
{
    double start = omp_get_wtime();


    double end = omp_get_wtime();
    double time = end-start;
    printf("Mode 4: TaskBasedParallelism. Processing time taken: %f\n", time);
}

void SIMDOnly()
{
    double start = omp_get_wtime();


    double end = omp_get_wtime();
    double time = end-start;
    printf("Mode 5: SIMDOnly. Processing time taken: %f\n", time);
}

void threadsAndSIMD()
{
    double start = omp_get_wtime();

    double end = omp_get_wtime();
    double time = end-start;
    printf("Mode 6: ThreadsAndSIMD. Processing time taken: %f\n", time);
}

int main(int argc, int** argv)
{

    //matrix sizes have three sizes: small/medium/large (512, 1024, 2048)
    int N = argv[1];
    createAandBMatrices(N);
    double C[] = double * malloc(sizeof(double) * N * N);

    switch (argv[2])
    {
        case 0:
        serial();
        break;

        case 1:
        openMP();
        break;

        case 2:
        openMPCollapse();
        break;

        case 3:
        synchronizationComparison();
        break;

        case 4:
        taskBasedParallelism();
        break;

        case 5:
        SIMDOnly();
        break;
        
        case 6:
        threadsAndSIMD();
        break;

        default:
        printf("\nError interpreting mode command. Please use ./A N Mode format.");
        printf("\nMode 0: Serial Baseline (No OpenMP at all)");
        printf("\nMode 1: OpenMP Threads (Work-sharing loops + correct scoping)");
        printf("\nMode 2: OpenMP Threads + collapse(2)");
        printf("\nMode 3: Synchronization Comparison (Atomic vs Critical)");
        printf("\nMode 4: Task-Based Parallelism");
        printf("\nMode 5: SIMD Only (no threading)");
        printf("\nMode 6: Threads + SIMD");
    }
}