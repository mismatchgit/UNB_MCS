#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 4){
          if (rank == 0) {
            printf("Usage: mpirun -np P ./integration func_id mode tol\n");
            printf("  func_id: 0, 1, or 2\n");
            printf("  mode   : 0=serial, 1=static MPI, 2=dynamic MPI\n");
            printf("  tol    : positive number like 1e-6\n");
        }
        MPI_Finalize();
        return 1;
    }

    int func_id = atoi(argv[1]);
    int mode = atoi(argv[2]);
    double tol = atof(argv[3]);

    if (func_id < 0 || func_id > 2 || mode < 0 || mode > 2 || tol <= 0.0) {
        if (rank == 0) {
            printf("Invalid arguments.\n");
            printf("Usage: mpirun -np P ./integration func_id mode tol\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        printf("Started with:\n");
        printf("processes = %d\n", size);
        printf("func_id = %d\n", func_id);
        printf("mode = %d\n", mode);
        printf("tol = %.12g\n", tol);
    }

    MPI_Finalize();
    return 0;
}