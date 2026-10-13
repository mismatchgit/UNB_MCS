#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_DEPTH 60
#define MIN_WIDTH 1e-15

double funcEval(int funcId, double x) {
    switch (funcId) {
        case 0:
            return sin(x) + 0.5 * cos(3.0 * x);
        case 1: {
            double d = x - 0.3;
            return 1.0 / (1.0 + 100.0 * d * d);
        }
        case 2:
            return sin(200.0 * x) * exp(-x);
        default:
            return 0.0;
    }
}

double simpsonEstimate(int funcId, double a, double b) {
    double m = 0.5 * (a + b);
    double f1 = funcEval(funcId, a);
    double f2 = funcEval(funcId, m);
    double f3 = funcEval(funcId, b);
    return (b - a) * (f1 + 4.0 * f2 + f3) / 6.0;
}

double adaptiveSimpson(int funcId, double a, double b, double tol, int depth, long long *acceptedTotal) {
    double m = 0.5 * (a + b);

    double s = simpsonEstimate(funcId, a, b);
    double sLeft = simpsonEstimate(funcId, a, m);
    double sRight = simpsonEstimate(funcId, m, b);
    double s2 = sLeft + sRight;

    double err = fabs(s2 - s);

    if (err <= 15.0 * tol || depth >= MAX_DEPTH || fabs(b - a) <= MIN_WIDTH) {
        (*acceptedTotal)++;
        return s2 + (s2 - s) / 15.0;
    }

    return adaptiveSimpson(funcId, a, m, tol / 2.0, depth + 1, acceptedTotal) + adaptiveSimpson(funcId, m, b, tol / 2.0, depth + 1, acceptedTotal);
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int myRank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    if (argc != 4) {
        if (myRank == 0) {
            printf("Usage: mpirun -np P ./integration funcId mode tol\n");
            printf("  funcId: 0, 1, or 2\n");
            printf("  mode   : 0=serial, 1=static MPI, 2=dynamic MPI\n");
            printf("  tol    : positive number like 1e-6\n");
        }
        MPI_Finalize();
        return 1;
    }
    int funcId = atoi(argv[1]);
    int mode = atoi(argv[2]);
    double tol = atof(argv[3]);
    if (funcId < 0 || funcId > 2 || mode < 0 || mode > 2 || tol <= 0.0) {
        if (myRank == 0) {
            printf("Invalid arguments.\n");
            printf("Usage: mpirun -np P ./integration funcId mode tol\n");
        }
        MPI_Finalize();
        return 1;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    if (mode == 0) {
        if (myRank == 0) {
            long long acceptedTotal = 0;
            double result = adaptiveSimpson(funcId, 0.0, 1.0, tol, 0, &acceptedTotal);

            MPI_Barrier(MPI_COMM_WORLD);
            double end = MPI_Wtime();
            printf("Serial mode\n");
            printf("funcId = %d\n", funcId);
            printf("tol = %.12g\n", tol);
            printf("integral = %.15f\n", result);
            printf("acceptedIntervals = %lld\n", acceptedTotal);
            printf("runtimeSeconds = %.9f\n", end - start);
        }
    } else {

    }

    MPI_Finalize();
    return 0;
}