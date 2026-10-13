#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define NUM_THREADS 8

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
        {maxC = C[i];}
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

    #pragma omp parallel for schedule(static) shared(A, B, C) private (i, j, k) //other version: schedule(dynamic, 10)
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
    //Compute sumC, maxC, checksum
    int sumC = 0;
    int maxC = 0;
    double checksum = 0;

    for (int i = 0; i < C.length; i++)
    {
        if (C[i] > maxC)
        {maxC = C[i];}
        sumC += C[i];
        checksum += (long long)(C[i * N + j] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = end-start;

    printf("Mode 1: OpenMP. Total Processing time taken: %f", time);
    printf("Mode 1: OpenMP. Sum of C: %d", time);
    printf("Mode 1: OpenMP. Max of C: %d", time);
    printf("Mode 1: OpenMP. Checksum of C: %f", time);
}

void openMPCollapse(int N)
{
    double startTotal = omp_get_wtime();
    //Compute AxB
    double startKernal = omp_get_wtime();

    int sumArray[N]; 
    int totalSum = 0;

    #pragma omp parallel for collapse(2)
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
    //Compute sumC, maxC, checksum
    int sumC = 0;
    int maxC = 0;
    double checksum = 0;

    #pragma omp parallel for
    for (int i = 0; i < C.length; i++)
    {
        if (C[i] > maxC)
        {maxC = C[i];}
        sumC += C[i];
        checksum += (long long)(C[i * N + j] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = end-start;
    
    printf("Mode 2: OpenMPCollapse. Processing time taken: %f", time);
    printf("Mode 2: OpenMPCollapse. Sum of C: %d", time);
    printf("Mode 2: OpenMPCollapse. Max of C: %d", time);
    printf("Mode 2: OpenMPCollapse. Checksum of C: %f", time);
}


void synchronizationComparison(int N)
{
    printf("\n~~~Initialize Part 3A~~~\n"); //atomic
    double startTotal3A = omp_get_wtime();
    //Compute AxB
    double startKernal3A = omp_get_wtime();

    int totalSum = 0;

    #pragma omp parallel for
    for(int i = 0; i < N; i++)
    {
        for(int j = 0 ; j < N; j++)
        {
            int totalSum = 0;

            for(int k = 0; k < N; k++)
            {
                #pragma omp atomic
                totalSum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = totalSum;
        }
    }

    double endKernal3A = omp_get_wtime();
    double timeKernal3A = endKernal3A - startKernal3A;
    printf("Mode 3A: SynchronSIMDization Comparison (atomic). Kernal Processing time taken: %f", timeKernal3A);
    //Compute sumC, maxC, checksum
    int sumC = 0;
    int maxC = 0;
    double checksum = 0;

    #pragma omp parallel for
    for (int i = 0; i < C.length; i++)
    {
        if (C[i] > maxC)
        {maxC = C[i];}
        sumC += C[i];
        checksum += (long long)(C[i * N + j] * 1000.0) % 100000;
    }

    double endTotal3A = omp_get_wtime();
    double timeTotal3A = end3A-start3A;
    printf("\nMode 3A: SynchronizationComparison (atomic). Processing time taken: %f", time3A);
    printf("\nMode 3A: SynchronizationComparison (atomic). Sum of C: %d", sumC);
    printf("\nMode 3A: SynchronizationComparison (atomic). Max of C: %d", maxC);
    printf("\nMode 3A: SynchronizationComparison (atomic). Checksum of C: %f", checksum);


    printf("\n~~~Initialize Part 3B~~~\n"); //critical

    double startTotal3B = omp_get_wtime();
    //Compute AxB
    double startKernal3B = omp_get_wtime();

    #pragma omp parallel for
    for(int i = 0; i < N; i++)
    {
        for(int j = 0 ; j < N; j++)
        {
            int totalSum = 0;

            for(int k = 0; k < N; k++)
            {
                #pragma omp critical
                {
                    totalSum += A[i * N + k] * B[k * N + j];
                }
            }
            C[i * N + j] = totalSum;
        }
    }

    double endKernal3B = omp_get_wtime();
    double timeKernal3B = endKernal3B - startKernal3B;
    printf("Mode 3B: Synchronization Comparison (critical). Kernal Processing time taken: %f", timeKernal3A);
    
    //Compute sumC, maxC, checksum
    sumC = 0;
    maxC = 0;
    checksum = 0;

    #pragma omp parallel for
    for (int i = 0; i < C.length; i++)
    {
        if (C[i] > maxC)
        {maxC = C[i];}
        sumC += C[i];
        checksum += (long long)(C[i * N + j] * 1000.0) % 100000;
    }

    double endTotal3B = omp_get_wtime();
    double timeTotal3B = end3B-start3B;
    printf("\nMode 3B: SynchronizationComparison (critical). Processing time taken: %f", time3B);
    printf("\nMode 3B: SynchronizationComparison (critical). Sum of C: %d", sumC);
    printf("\nMode 3B: SynchronizationComparison (critical). Max of C: %d", maxC);
    printf("\nMode 3B: SynchronizationComparison (critical). Checksum of C: %f", checksum);
}

void taskBasedParallelism(int N)
{
    double startTotal = omp_get_wtime();
    //Compute AxB
    double startKernal = omp_get_wtime();

    int sumArray[N]; 
    int totalSum = 0;

    #pragma omp parallel for collapse(2)
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
    //Compute sumC, maxC, checksum
    int sumC = 0;
    int maxC = 0;
    double checksum = 0;

    #pragma omp parallel for
    for (int i = 0; i < C.length; i++)
    {
        if (C[i] > maxC)
        {maxC = C[i];}
        sumC += C[i];
        checksum += (long long)(C[i * N + j] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = end-start;
    
    printf("Mode 4: TaskBasedParallelism. Processing time taken: %f", time);
    printf("Mode 4: TaskBasedParallelism. Sum of C: %d", time);
    printf("Mode 4: TaskBasedParallelism. Max of C: %d", time);
    printf("Mode 4: TaskBasedParallelism. Checksum of C: %f", time);
}

void SIMDOnly()
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

            int totalSum = 0;

            #pragma omp simd reduction(+:totalSum)
            for(int k = 0; k < N; k++)
            {
                totalSum += A[i * N + k] * B[k * N + j];
            }

            C[i * N + j] = totalSum;
        }
    }

    double endKernal = omp_get_wtime();
    double timeKernal = endKernal - startKernal;
    printf("Mode 5: SIMDOnly. Kernal Processing time taken: %f", timeKernal);
    //Compute sumC, maxC, checksum
    int sumC = 0;
    int maxC = 0;
    double checksum = 0;

    for (int i = 0; i < C.length; i++)
    {
        if (C[i] > maxC)
        {maxC = C[i];}
        sumC += C[i];
        checksum += (long long)(C[i * N + j] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = end-start;

    printf("Mode 5: SIMDOnly. Total Processing time taken: %f", time);
    printf("Mode 5: SIMDOnly. Sum of C: %d", time);
    printf("Mode 5: SIMDOnly. Max of C: %d", time);
    printf("Mode 5: SIMDOnly. Checksum of C: %f", time);
}

void threadsAndSIMD()
{
    double startTotal = omp_get_wtime();
    //Compute AxB
    double startKernal = omp_get_wtime();

    int sumArray[N]; 
    int totalSum = 0;

    #pragma omp parallel for schedule(static)
    for(int i = 0; i < N; i++)
    {
        for(int j = 0 ; j < N; j++)
        {

            int totalSum = 0;

            #pragma omp simd reduction(+:totalSum)
            for(int k = 0; k < N; k++)
            {
                totalSum += A[i * N + k] * B[k * N + j];
            }

            C[i * N + j] = totalSum;
        }
    }

    double endKernal = omp_get_wtime();
    double timeKernal = endKernal - startKernal;
    printf("Mode 6: ThreadsAndSIMD. Kernal Processing time taken: %f", timeKernal);

    //Compute sumC, maxC, checksum
    int sumC = 0;
    int maxC = 0;
    double checksum = 0;

    for (int i = 0; i < C.length; i++)
    {
        if (C[i] > maxC)
        {maxC = C[i];}
        sumC += C[i];
        checksum += (long long)(C[i * N + j] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = end-start;

    printf("Mode 6: ThreadsAndSIMD. Total Processing time taken: %f", time);
    printf("Mode 6: ThreadsAndSIMD. Sum of C: %d", time);
    printf("Mode 6: ThreadsAndSIMD. Max of C: %d", time);
    printf("Mode 6: ThreadsAndSIMD. Checksum of C: %f", time);
}

int main(int argc, int** argv) //N size, 
{
    omp_set_num_threads(NUM_THREADS);
    //matrix sizes have three sizes: small/medium/large (512, 1024, 2048)
    int N = argv[1];

    if (N != 512 || N != 1024 || N != 2048)
    {
        printf("Please choose a correct size of N: 512, 1024, 2048.");
        return 1;
    }

    createAandBMatrices(N);
    double C[] = double * malloc(sizeof(double) * N * N);

    switch (argv[2])
    {
        case 0:
            serial(N);
        break;

        case 1:
            openMP(N);
        break;

        case 2:
            openMPCollapse(N);
        break;

        case 3:
            synchronizationComparison(N);
        break;

        case 4:
            taskBasedParallelism(N);
        break;

        case 5:
            SIMDOnly(N);
        break;
        
        case 6:
            threadsAndSIMD(N);
        break;

        default:
        printf("\nError interpreting mode command. Please use ./A [N] [Mode] format.");
        printf("\nSizes Available: 512, 1024, 2048.\n");
        printf("\nModes Available:"); 
        printf("\nMode 0: Serial Baseline (No OpenMP at all)");
        printf("\nMode 1: OpenMP Threads (Work-sharing loops + correct scoping)");
        printf("\nMode 2: OpenMP Threads + collapse(2)");
        printf("\nMode 3: Synchronization Comparison (Atomic vs Critical)");
        printf("\nMode 4: Task-Based Parallelism");
        printf("\nMode 5: SIMD Only (no threading)");
        printf("\nMode 6: Threads + SIMD");
    }
}