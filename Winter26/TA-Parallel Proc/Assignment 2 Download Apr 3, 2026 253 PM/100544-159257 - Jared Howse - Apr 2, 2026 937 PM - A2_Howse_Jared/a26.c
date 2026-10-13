#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_DEPTH 60
#define MIN_WIDTH 1e-15


double func_eval(int func_id, double x){
    switch(func_id){
        case 0: {
            return sin(x) + 0.5 * cos (3.0 * x);
        }
        case 1: {
            double d = x - 0.3;
            return 1.0 / (1.0 + 100.0 * d * d);
        }
        case 2: {
            return sin(200.0 * x) * exp(-x);
        }
        default:
            return 0.0;
    }
}

double simpson_estimate(int func_id, double a, double b) {
    double m = 0.5 * (a + b);
    double f1 = func_eval(func_id, a);
    double f2 = func_eval(func_id, m);
    double f3 = func_eval(func_id, b);
    return (b - a) * (f1 + 4.0 * f2 + f3) / 6.0;
}

double adaptive_simpson(int func_id, double a, double b, double tol, int depth, long long *accepted_total) {
    double m = 0.5 * (a + b);

    double s = simpson_estimate(func_id, a, b);
    double s_left = simpson_estimate(func_id, a, m);
    double s_right = simpson_estimate(func_id, m, b);
    double s2 = s_left + s_right;

    double err = fabs(s2 - s);

    if (err <= 15.0 * tol || depth >= MAX_DEPTH || fabs(b - a) <= MIN_WIDTH) {
        (*accepted_total)++;
        return s2 + (s2 - s) / 15.0;
    }

    return adaptive_simpson(func_id, a, m, tol / 2.0, depth + 1, accepted_total) + adaptive_simpson(func_id, m, b, tol / 2.0, depth + 1, accepted_total);
}


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

    
    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    double result = 0.0;
    long long accepted_total = 0;
    int chosen_k = 0;
    long long local_accepted = 0;
    long long min_local = 0;
    long long max_local = 0;

    if (mode == 0) {
        if (rank == 0) {
            result = adaptive_simpson(func_id, 0.0, 1.0, tol, 0, &accepted_total);
        }
    }
    else if (mode == 1) {

    }
    else {
        if (rank == 0) {
            printf("Mode 2 not implemented yet.\n");
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();

    if (rank == 0) {
        printf("func_id = %d\n", func_id);
        printf("mode = %d\n", mode);
        printf("tol = %.12g\n", tol);
        printf("integral = %.15f\n", result);
        printf("accepted_intervals = %lld\n", accepted_total);
        printf("runtime_seconds = %.6f\n", end - start);
    }

    MPI_Finalize();
    return 0;
}