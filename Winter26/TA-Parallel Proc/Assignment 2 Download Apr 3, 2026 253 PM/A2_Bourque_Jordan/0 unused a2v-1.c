#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

double function0()
{
//𝑓(𝑥)=sin⁡(𝑥)+0.5cos⁡(3𝑥)
}

double function1()
{
//𝑓(𝑥)=1/(1+100(𝑥−0.3)^2)
}

double function2()
{
//𝑓(𝑥)=sin⁡(200𝑥)𝑒−𝑥
}

int main(int argc, char** argv) 
{

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 4) 
    {
        if (rank == 0)
        {
            printf("Usage: %s <Number of processes>, <function (0, 1, 2)>, <mode>, <tol>\n", argv[0]);
        }
        MPI_Finalize();
        return 0;
    }

    int P = atoi(argv[1]);
    int func_id = atoi(argv[2]);
    int mode = atoi(argv[3]);
    int tol = atoi(argv[4]);

    double *local_A = (double*) malloc(local_rows * N * sizeof(double));
    double *local_y = (double*) malloc(local_rows * sizeof(double));
    double *x = (double*) malloc(N * sizeof(double));

    double *A = NULL;
    double *y = NULL;

    if (rank == 0) 
    {
        A = (double*) malloc(N * N * sizeof(double));
        y = (double*) malloc(N * sizeof(double));

        for (int i = 0; i < N * N; i++)
        {
            A[i] = 1.0;
        }
        for (int i = 0; i < N; i++)
        {
            x[i] = 1.0;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    MPI_Scatter(A, local_rows * N, MPI_DOUBLE, local_A, local_rows *N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Bcast(x, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    for (int i = 0; i < local_rows; i++)
    {
        local_y[i] = 0.0;
        for (int j = 0; j < N; j++)
        {
            local_y[i] += local_A[i*N + j] * x[j];
        }
    }

    MPI_Gather(local_y, local_rows, MPI_DOUBLE, y, local_rows, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    if (rank == 0) 
    {
        printf("N = %d, Processes = %d, Time = %f seconds\n", N, size, end_time - start_time);
        free(A);
        free(y);
    }

    free(local_A);
    free(local_y);
    free(x);

    MPI_Finalize();
    return 0;
}