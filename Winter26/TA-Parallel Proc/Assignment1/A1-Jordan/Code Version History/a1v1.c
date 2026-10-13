#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <omp.h>


bool checksumVerification(int[] C)
{
    int checksum = 0;

    for()
    {
        checksum += (long long)(C[i*N + j] * 1000.0) % 100000;
    }
    if (checksum) //some if statement
    {
        return true;
    }
    else return false;


}

int serial()
{

}

int openMP()
{

}

int openMPCollapse()
{

}

int taskBasedParallelism()
{

}

int SIMDOnly()
{

}

int threadsAndSIMD()
{

}

int main(int argc, int** argv)
{

    switch (argv[2]){
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