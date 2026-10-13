#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

    if (myRank == 0) {
        printf("Started with:\n");
        printf("processes = %d\n", numProcs);
        printf("funcId = %d\n", funcId);
        printf("mode = %d\n", mode);
        printf("tol = %.8g\n", tol);
        printf("\nSample function values for testing the program at current state!\n");
        printf("f(0.0) = %.8f\n", funcEval(funcId, 0.0));
        printf("f(0.5) = %.8f\n", funcEval(funcId, 0.5));
        printf("f(1.0) = %.8f\n", funcEval(funcId, 1.0));

        double whole = simpsonEstimate(funcId, 0.0, 1.0);
        double left = simpsonEstimate(funcId, 0.0, 0.5);
        double right = simpsonEstimate(funcId, 0.5, 1.0);

        printf("\nSimpson estimates:\n");
        printf("S(0.0, 1.0)   = %.8f\n", whole);
        printf("S(0.0, 0.5)   = %.8f\n", left);
        printf("S(0.5, 1.0)   = %.8f\n", right);
        printf("Left + Right  = %.8f\n", left + right);
    }

    MPI_Finalize();
    return 0;
}