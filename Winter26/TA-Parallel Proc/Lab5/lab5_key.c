#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) {
            printf("Usage: %s <matrix_size_N>\n", argv[0]);
        }
        MPI_Finalize();
        return 0;
    }

    int N = atoi(argv[1]);
    if (N <= 0) {
        if (rank == 0) {
            printf("Error: N must be a positive integer.\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Compute rows per process (supports N not divisible by size)
    int base_rows = N / size;
    int remainder = N % size;

    int local_rows = base_rows + (rank < remainder ? 1 : 0);
    int local_elements = local_rows * N;

    // Allocate local data
    double *local_A = NULL;
    double *local_y = NULL;
    double *x = NULL;

    if (local_rows > 0) {
        local_A = (double *)malloc((size_t)local_elements * sizeof(double));
        local_y = (double *)malloc((size_t)local_rows * sizeof(double));
    } else {
        // still allowed if some ranks get zero rows
        local_A = NULL;
        local_y = NULL;
    }

    x = (double *)malloc((size_t)N * sizeof(double));

    if ((local_rows > 0 && (!local_A || !local_y)) || !x) {
        fprintf(stderr, "Rank %d: local memory allocation failed.\n", rank);
        free(local_A);
        free(local_y);
        free(x);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Root allocates full matrix and result vector
    double *A = NULL;
    double *y = NULL;
    int *sendcounts = NULL;
    int *displs = NULL;
    int *recvcounts_y = NULL;
    int *displs_y = NULL;

    if (rank == 0) {
        // Print approximate memory requirements
        double matrix_gb = ((double)N * (double)N * sizeof(double)) / (1024.0 * 1024.0 * 1024.0);
        double vector_mb = ((double)N * sizeof(double)) / (1024.0 * 1024.0);
        printf("Requested matrix size N = %d\n", N);
        printf("Approx memory for full dense matrix A on rank 0 = %.2f GB\n", matrix_gb);
        printf("Approx memory for one vector of size N = %.2f MB\n", vector_mb);

        A = (double *)malloc((size_t)N * (size_t)N * sizeof(double));
        y = (double *)malloc((size_t)N * sizeof(double));

        sendcounts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
        recvcounts_y = (int *)malloc(size * sizeof(int));
        displs_y = (int *)malloc(size * sizeof(int));

        if (!A || !y || !sendcounts || !displs || !recvcounts_y || !displs_y) {
            fprintf(stderr, "Rank 0: memory allocation failed.\n");
            free(A);
            free(y);
            free(sendcounts);
            free(displs);
            free(recvcounts_y);
            free(displs_y);
            free(local_A);
            free(local_y);
            free(x);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Initialize A and x
        for (int i = 0; i < N * N; i++) {
            A[i] = 1.0;
        }
        for (int i = 0; i < N; i++) {
            x[i] = 1.0;
        }

        // Prepare Scatterv / Gatherv metadata
        int offsetA = 0;
        int offsetY = 0;
        for (int p = 0; p < size; p++) {
            int rows_p = base_rows + (p < remainder ? 1 : 0);
            sendcounts[p] = rows_p * N;   // elements of A
            displs[p] = offsetA;
            offsetA += sendcounts[p];

            recvcounts_y[p] = rows_p;     // elements of y
            displs_y[p] = offsetY;
            offsetY += recvcounts_y[p];
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    // Broadcast x to everyone
    MPI_Bcast(x, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Scatter matrix rows
    MPI_Scatterv(A, sendcounts, displs, MPI_DOUBLE,
                 local_A, local_elements, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    // Local matrix-vector multiplication
    for (int i = 0; i < local_rows; i++) {
        local_y[i] = 0.0;
        for (int j = 0; j < N; j++) {
            local_y[i] += local_A[i * N + j] * x[j];
        }
    }

    // Gather result
    MPI_Gatherv(local_y, local_rows, MPI_DOUBLE,
                y, recvcounts_y, displs_y, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    if (rank == 0) {
        printf("N = %d, Processes = %d, Time = %f seconds\n", N, size, end_time - start_time);

        // Optional correctness check
        int correct = 1;
        for (int i = 0; i < N; i++) {
            if (y[i] != (double)N) {
                correct = 0;
                printf("Mismatch at y[%d] = %f, expected %f\n", i, y[i], (double)N);
                break;
            }
        }

        if (correct) {
            printf("Result is correct. Each y[i] = %d\n", N);
        }

        free(A);
        free(y);
        free(sendcounts);
        free(displs);
        free(recvcounts_y);
        free(displs_y);
    }

    free(local_A);
    free(local_y);
    free(x);

    MPI_Finalize();
    return 0;
}