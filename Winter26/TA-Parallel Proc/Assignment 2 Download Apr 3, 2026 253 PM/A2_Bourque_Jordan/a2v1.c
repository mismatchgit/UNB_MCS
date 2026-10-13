#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int runTimes[10];

double function0(double x)
{
    return sin(x) + 0.5 * cos(3x);
}

double function1(double x)
{
    return 1/(1+100(x-0.3)^2);
}

double function2(double x)
{
    return (sin(200x)e-x);
}

double simpsonApprox()
{

    return (b-a)/6()

}

int serialBaseline()
{
    
}

int staticSched()
{

}

int dynamicShed()
{

}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 5)
    {
        if(rank == 0)
        {
            printf("\nPlease enter P, func_id, mode, and tol.");
        }
        MPI_Finalize();
        return 1;
    }

    int P           = atoi(argv[1]);
    int func_id     = atoi(argv[2]);
    int mode        = atoi(argv[3]);
    int tol         = atoi(argv[4]);

    for(int i = 0; i <= sizeOf(runTimes); i++)
    {
        MPI_Barrier(MPI_COMM_WORLD);
        double start = MPI_Wtime();

        Image *output = NULL;

        if(mode == 0)
        {
            if(rank == 0)
            {
                result = serialBaseline(func_id, tol, rank, size);
            }
        }
        if(mode == 1)
        {
            if(rank == 0)
            {
                result = staticSched(func_id, tol, rank, size);
            }
        }        
        if(mode == 2)
        {
            if(rank == 0)
            {
                result = dynamicSched(func_id, tol, rank, size);
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        double end = MPI_Wtime();

        if(rank == 0)
        {
            double totalTime = end - start;
            runTimes[i] = totalTime;
            printf("\nRun %d Mode %d, Processing time taken: %.9f", i, mode, totalTime);
            fflush(stdout);
        }

        MPI_Gatherv(
            output->data,
            recvcounts[rank],
            MPI_UNSIGNED_CHAR,
            gathered,
            recvcounts,
            recvdisplacement,
            MPI_UNSIGNED_CHAR,
            0,
            MPI_COMM_WORLD
        );
    }


    if(rank == 0)
    {
        double absoluteTime;
        for (int i = 0; i < sizeOf(runTimes); i++)
        {
            absoluteTime += runTimes[i];
        }
        printf("\n\nMode %d, Average Processing time taken: %.9f", mode, totalTime);
    }

    MPI_Finalize();
    return 0;
}