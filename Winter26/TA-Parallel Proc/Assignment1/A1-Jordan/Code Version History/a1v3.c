#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <omp.h>

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

void serial(int N)
{
    double startTotal = omp_get_wtime();
    //Compute AxB
    double startKernal = omp_get_wtime();

    int sumArray[N]; 
    int totalSum = 0;

    for(int i = 0; i < N; i++)
    {
        for(int j = 0 ; j < N; j++)
        {
            totalSum = 0;
            sumArray[j] = A[i] * B[j * N + i];

            for (int k = 0; k < N; k++)
            {
                totalSum += sumArray[k];
            }

            C[i * N + j] = totalSum; //assign multiplied value into new array

            for(int l = 0; l < N; l++)
            {
                sumArray[k] = 0; //erase all values in the sumArray;
            }
        }
    }

    double endKernal = omp_get_wtime();
    double timeKernal = endKernal - startKernal;
    printf("Mode 0: Serial. Kernal Processing time taken: %f\n", timeKernal);
    //Compute sumC
    int sumC = 0;
    int maxC = 0;
    double checksum = 0;
    
    for (int i = 0; i < C.length; i++)
    {
        sumC += C[i];
    }

    //Compute maxC
    for (int i = 0; i < C.length; i++)
    {
        if (C[i] > maxC)
        {
            maxC = C[i];
        }
    }

    //Compute checksum
    for(int i = 0; i < C.length; i++)
    {
        checksum += (long long)(C[i * N + j] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = end-start;
    printf("\nMode 0: Serial. Total Processing time taken: %f", timeTotal);
    printf("\nMode 0: Serial. Sum of C: %d", sumC);
    printf("\nMode 0: Serial. Max of C: %d", maxC);
    printf("\nMode 0: Serial. Checksum of C: %f", checksum);
}

void openMP(int N)
{
    double startTotal = omp_get_wtime();
    //Compute AxB
    double startKernal = omp_get_wtime();

    int sumArray[N]; 
    int totalSum = 0;

    #pragma omp parallel for collapse (2) schedule(static) shared(A, B, C) private (i, j, k) //other version: schedule(dynamic, 10)
    for(int i = 0; i < N; i++)
    {
        for(int j = 0 ; j < N; j++)
        {

            int totalSum = 0;

            for(int k = 0; k < N; k++)
            {
                totalSum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = totalSum;
        }
    }

    double endKernal = omp_get_wtime();
    double timeKernal = endKernal - startKernal;
    printf("Mode 1: OMP. Kernal Processing time taken: %f", timeKernal);
    //Compute sumC
    int sumC = 0;
    int maxC = 0;
    double checksum = 0;
    
    for (int i = 0; i < C.length; i++)
    {
        sumC += C[i];
    }

    //Compute maxC
    for (int i = 0; i < C.length; i++)
    {
        if (C[i] > maxC)
        {
            maxC = C[i];
        }
    }

    //Compute checksum
    for(int i = 0; i < C.length; i++)
    {
        checksum += (long long)(C[i * N + j] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = end-start;
    printf("\nMode 0: Serial. Total Processing time taken: %f", timeTotal);
    printf("\nMode 0: Serial. Sum of C: %d", sumC);
    printf("\nMode 0: Serial. Max of C: %d", maxC);
    printf("\nMode 0: Serial. Checksum of C: %f", checksum);

    printf("Mode 1: OpenMP. Total Processing time taken: %f", time);
    printf("Mode 1: OpenMP. Sum of C: %d", time);
    printf("Mode 1: OpenMP. Max of C: %f", time);
    printf("Mode 1: OpenMP. Checksum of C: %f", time);
}

void openMPCollapse()
{
    double startTotal = omp_get_wtime();
    //Compute AxB
    double startKernal = omp_get_wtime();

    int sumArray[N]; 
    int totalSum = 0;

    for(int i = 0; i < N; i++)
    {
        for(int j = 0 ; j < N; j++)
        {
            totalSum = 0;
            sumArray[j] = A[i] * B[j * N + i];

            for (int l = 0; l < N; l++)
            {
                totalSum += sumArray[l];
            }

            C[i * N + j] = totalSum; //assign multiplied value into new array

            for(int k = 0; k < N; k++)
            {
                sumArray[k] = 0; //erase all values in the sumArray;
            }
        }
    }

    double endKernal = omp_get_wtime();
    double timeKernal = endKernal - startKernal;
    printf("Mode 0: Serial. Kernal Processing time taken: %f\n", timeKernal);
    //Compute sumC
    int sumC = 0;
    int maxC = 0;
    double checksum = 0;
    
    for (int i = 0; i < C.length; i++)
    {
        sumC += C[i];
    }

    //Compute maxC
    for (int i = 0; i < C.length; i++)
    {
        if (C[i] > maxC)
        {
            maxC = C[i];
        }
    }

    //Compute checksum
    for(int i = 0; i < C.length; i++)
    {
        checksum += (long long)(C[i * N + j] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = end-start;
    printf("Mode 2: OpenMPCollapse. Processing time taken: %f", time);
    printf("Mode 2: OpenMPCollapse. Sum of C: %f", time);
    printf("Mode 2: OpenMPCollapse. Max of C: %f", time);
    printf("Mode 2: OpenMPCollapse. Checksum of C: %f", time);
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