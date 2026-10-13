#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0)
            printf("Usage: %s <matrix_size_N>\n", argv[0]);
        MPI_Finalize();
        return 0;
    }

    int N = atoi(argv[1]);
    int local_rows = N / size;

    double *local_A = (double*) malloc(local_rows * N * sizeof(double));
    double *local_y = (double*) malloc(local_rows * sizeof(double));
    double *x = (double*) malloc(N * sizeof(double));

    double *A = NULL;
    double *y = NULL;

    if (rank == 0) {

        A = (double*) malloc(N * N * sizeof(double));
        y = (double*) malloc(N * sizeof(double));

        for (int i = 0; i < N*N; i++)
            A[i] = 1.0;

        for (int i = 0; i < N; i++)
            x[i] = 1.0;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    /* TODO
       1. Scatter matrix A to local_A
    */
    MPI_Scatter(A, // send buffer
      local_rows * N, // send count
      MPI_DOUBLE, // send type
      local_A, // receive buffer
      local_rows * N, // receive count
      MPI_DOUBLE, // receive type
      0, // root process
      MPI_COMM_WORLD // communicator
    );

    /* TODO
       2. Broadcast vector x of count N to all processes from root process 0
    */
    MPI_Bcast(x, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    /* TODO
       3. Compute local matrix-vector product
        local_y = local_A * x
    */
    for (int i = 0; i < local_rows; i++) {
        local_y[i] = 0.0;
        for (int j = 0; j < N; j++) {
          // local_A is stored in row-major order, so the element at row i and column j is at index i * N + j
            local_y[i] += local_A[i * N + j] * x[j];
        }
    }

    /* TODO
       4. Gather results into y
    */
    MPI_Gather(local_y, // send buffer
      local_rows, // send count
      MPI_DOUBLE, // send type
      y, // receive buffer 
      local_rows, // receive count
      MPI_DOUBLE, // receive type
      0, // root process
      MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    if (rank == 0) {
        printf("N = %d, Processes = %d, Time = %f seconds\n",
               N, size, end_time - start_time);
        free(A);
        free(y);
    }

    free(local_A);
    free(local_y);
    free(x);

    MPI_Finalize();
    return 0;
}
