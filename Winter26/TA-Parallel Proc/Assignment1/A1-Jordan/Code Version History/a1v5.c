#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define NUM_THREADS 8

double *A, *B, *C;

void createAandBMatrices(int N)
{
    A = malloc(sizeof(double) * N * N);
    B = malloc(sizeof(double) * N * N);

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

    for(int i = 0; i < N; i++)
    {
        for(int j = 0; j < N; j++)
        {
            double totalSum = 0.0;

            for(int k = 0; k < N; k++)
            {
                totalSum += A[i * N + k] * B[k * N + j];
            }

            C[i * N + j] = totalSum;
        }
    }

    double endKernal = omp_get_wtime();
    double timeKernal = endKernal - startKernal;
    printf("Mode 0: Serial. Kernal Processing time taken: %f\n", timeKernal);
    //Compute sumC
    double sumC = 0.0;
    double maxC = C[0];
    long long checksum = 0;
    
    for (int i = 0; i < N*N; i++)
    {
        sumC += C[i];
    }

    //Compute maxC
    for (int i = 0; i < N*N; i++)
    {
        if (C[i] > maxC)
        {maxC = C[i];}
    }

    //Compute checksum
    for(int i = 0; i < N*N; i++)
    {
        checksum += (long long)(C[i] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = endTotal-startTotal;

    printf("\nMode 0: Serial. Total Processing time taken: %f", timeTotal);
    printf("\nMode 0: Serial. Sum of C: %f", sumC);
    printf("\nMode 0: Serial. Max of C: %f", maxC);
    printf("\nMode 0: Serial. Checksum of C: %lld", checksum);
}

void openMP(int N)
{
    double startTotal = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        printf("Threads used: %d\n", omp_get_num_threads());
    }
    //Compute AxB
    double startKernal = omp_get_wtime();

    #pragma omp parallel for schedule(static) //other version: schedule(dynamic, 10)
    for(int i = 0; i < N; i++)
    {
        for(int j = 0 ; j < N; j++)
        {

            double totalSum = 0.0;

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
    double sumC = 0.0;
    double maxC = C[0];
    long long checksum = 0;

    for (int i = 0; i < N*N; i++)
    {
        if (C[i] > maxC)
        {maxC = C[i];}
        sumC += C[i];
        checksum += (long long)(C[i] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = endTotal-startTotal;

    printf("Mode 1: OpenMP. Total Processing time taken: %f", timeTotal);
    printf("Mode 1: OpenMP. Sum of C: %f", sumC);
    printf("Mode 1: OpenMP. Max of C: %f", maxC);
    printf("Mode 1: OpenMP. Checksum of C: %lld", checksum);
}

void openMPCollapse(int N)
{
    double startTotal = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        printf("Threads used: %d\n", omp_get_num_threads());
    }
    //Compute AxB
    double startKernal = omp_get_wtime();

    #pragma omp parallel for collapse(2)
    for(int i = 0; i < N; i++)
    {
        for(int j = 0 ; j < N; j++)
        {

            double totalSum = 0.0;

            for(int k = 0; k < N; k++)
            {
                totalSum += A[i * N + k] * B[k * N + j];
            }

            C[i * N + j] = totalSum;
        }
    }

    double endKernal = omp_get_wtime();
    double timeKernal = endKernal - startKernal;
    printf("Mode 2: OpenMPCollapse. Kernal Processing time taken: %f", timeKernal);
    //Compute sumC, maxC, checksum
    double sumC = 0.0;
    double maxC = C[0];
    long long checksum = 0;

    #pragma omp parallel for
    for (int i = 0; i < N*N; i++)
    {
        if (C[i] > maxC)
        {maxC = C[i];}
        sumC += C[i];
        checksum += (long long)(C[i] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = endTotal-startTotal;
    
    printf("Mode 2: OpenMPCollapse. Processing time taken: %f", timeTotal);
    printf("Mode 2: OpenMPCollapse. Sum of C: %f", sumC);
    printf("Mode 2: OpenMPCollapse. Max of C: %f", maxC);
    printf("Mode 2: OpenMPCollapse. Checksum of C: %lld", checksum);
}


void synchronizationComparison(int N)
{
    double startTotal = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        printf("Threads used: %d\n", omp_get_num_threads());
    }
    //Compute AxB
    double startKernal = omp_get_wtime();

    #pragma omp parallel for
    for(int i = 0; i < N; i++)
    {
        for(int j = 0 ; j < N; j++)
        {
            double totalSum = 0.0;

            for(int k = 0; k < N; k++)
            {
                totalSum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = totalSum;
        }
    }

    double endKernal = omp_get_wtime();
    double timeKernal = endKernal - startKernal;
    printf("Mode 3: Synchronization Comparison. Kernal Processing time taken: %f", timeKernal);
    //Compute sumC, maxC, checksum
    double sumC = 0.0;
    double maxC = C[0];

    for(int i = 0; i < N*N; i++)
    {
        sumC += C[i];
        if(C[i] > maxC)
        {
            maxC = C[i];
        }
    }

    double endTotal = omp_get_wtime();
    double timeTotal = endTotal-startTotal;

    printf("\nMode 3: SynchronizationComparison. Processing time taken: %f", timeTotal);
    printf("\nMode 3: SynchronizationComparison. Sum of C: %f", sumC);
    printf("\nMode 3: SynchronizationComparison. Max of C: %f", maxC);  

    long long checksum = 0;
    long long checksum2 = 0;

    double checksum3AStartTime = omp_get_wtime();
    #pragma omp parallel for
    for(int i = 0; i < N * N; i++)
    {
        long long temp = (long long)(C[i] * 1000.0) % 100000;

        #pragma omp atomic
        checksum += temp;
    }
    double checksum3AEndTime = omp_get_wtime();
    double checksum3ATotalTime = checksum3AEndTime - checksum3AStartTime;
    printf("\nMode 3A: SynchronizationComparison. Checksum (atomic) Processing time taken: %f", checksum3ATotalTime);
    printf("\nMode 3A: SynchronizationComparison (atomic). Checksum of C: %lld", checksum);

    double checksum3BStartTime = omp_get_wtime();
    #pragma omp parallel for
    for(int i = 0; i < N * N; i++)
    {
        long long temp = (long long)(C[i] * 1000.0) % 100000;

        #pragma omp critical
        {
            checksum2 += temp;
        }
    }
    double checksum3BEndTime = omp_get_wtime();
    double checksum3BTotalTime = checksum3BEndTime - checksum3BStartTime;
    printf("\nMode 3B: SynchronizationComparison. Checksum (critical) Processing time taken: %f", checksum3BTotalTime);
    printf("\nMode 3B: SynchronizationComparison (critical). Checksum of C: %lld", checksum2);
}

void taskBasedParallelism(int N)
{
    double startTotal = omp_get_wtime();

    //Compute AxB
    double startKernal = omp_get_wtime();
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("Threads used: %d\n", omp_get_num_threads());
            #pragma omp taskgroup
            {
                for(int i = 0; i < N; i++)
                {
                    #pragma omp task firstprivate(i)
                    {
                        for(int j = 0 ; j < N; j++)
                        {
                            double totalSum = 0;
                            for(int k = 0; k < N; k++)
                            {
                                totalSum += A[i * N + k] * B[k * N + j];
                            }

                            C[i * N + j] = totalSum;
                        }
                    }
                }
            }
        }
    }
    

    double endKernal = omp_get_wtime();
    double timeKernal = endKernal - startKernal;
    printf("Mode 4: TaskBasedParallelism. Kernal Processing time taken: %f", timeKernal);
    //Compute sumC, maxC, checksum
    double sumC = 0.0;
    double maxC = C[0];
    long long checksum = 0;

    #pragma omp parallel for
    for (int i = 0; i < N*N; i++)
    {
        if (C[i] > maxC)
        {maxC = C[i];}
        sumC += C[i];
        checksum += (long long)(C[i] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = endTotal-startTotal;
    
    printf("Mode 4: TaskBasedParallelism. Processing time taken: %f", timeTotal);
    printf("Mode 4: TaskBasedParallelism. Sum of C: %f", sumC);
    printf("Mode 4: TaskBasedParallelism. Max of C: %f", maxC);
    printf("Mode 4: TaskBasedParallelism. Checksum of C: %lld", checksum);
}

void SIMDOnly(int N)
{
    omp_set_num_threads(1);
    double startTotal = omp_get_wtime();
    printf("Threads used: %d\n", omp_get_num_threads());
    //Compute AxB
    double startKernal = omp_get_wtime();

    for(int i = 0; i < N; i++)
    {
        for(int j = 0 ; j < N; j++)
        {

            double totalSum = 0.0;

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
    double sumC = 0.0;
    double maxC = C[0];
    long long checksum = 0;

    for (int i = 0; i < N*N; i++)
    {
        if (C[i] > maxC)
        {maxC = C[i];}
        sumC += C[i];
        checksum += (long long)(C[i] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = endTotal-startTotal;

    printf("Mode 5: SIMDOnly. Total Processing time taken: %f", timeTotal);
    printf("Mode 5: SIMDOnly. Sum of C: %f", sumC);
    printf("Mode 5: SIMDOnly. Max of C: %f", maxC);
    printf("Mode 5: SIMDOnly. Checksum of C: %lld", checksum);
}

void threadsAndSIMD(int N)
{
    double startTotal = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        printf("Threads used: %d\n", omp_get_num_threads());
    }
    //Compute AxB
    double startKernal = omp_get_wtime();

    #pragma omp parallel for schedule(static)
    for(int i = 0; i < N; i++)
    {
        for(int j = 0 ; j < N; j++)
        {

            double totalSum = 0.0;

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
    double sumC = 0.0;
    double maxC = C[0];
    long long checksum = 0;

    for (int i = 0; i < N*N; i++)
    {
        if (C[i] > maxC)
        {maxC = C[i];}
        sumC += C[i];
        checksum += (long long)(C[i] * 1000.0) % 100000;
    }

    double endTotal = omp_get_wtime();
    double timeTotal = endTotal-startTotal;

    printf("Mode 6: ThreadsAndSIMD. Total Processing time taken: %f", timeTotal);
    printf("Mode 6: ThreadsAndSIMD. Sum of C: %f", sumC);
    printf("Mode 6: ThreadsAndSIMD. Max of C: %f", maxC);
    printf("Mode 6: ThreadsAndSIMD. Checksum of C: %lld", checksum);
}

int main(int argc, char** argv) //N size, 
{
    if (argc != 3)
    {
        printf("Three arguments not presented for compute.");
        return 1;
    }

    omp_set_num_threads(NUM_THREADS);
    //matrix sizes have three sizes: small/medium/large (512, 1024, 2048)
    int N = atoi(argv[1]);
    int mode = atoi(argv[2]);

    if (N != 512 && N != 1024 && N != 2048)
    {
        printf("Please choose a correct size of N: 512, 1024, 2048.");
        return 1;
    }

    createAandBMatrices(N);
    C = malloc(sizeof(double) * N * N);

    switch (mode)
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

    free(A);
    free(B);
    free(C);
    return 0;
}