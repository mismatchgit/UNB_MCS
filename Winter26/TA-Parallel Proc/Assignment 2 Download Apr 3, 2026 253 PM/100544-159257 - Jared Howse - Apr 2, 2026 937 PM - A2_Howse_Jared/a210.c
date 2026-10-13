#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_DEPTH 60
#define MIN_WIDTH 1e-15

typedef struct {
    double a;
    double b;
    double tol;
    int depth;
} Task;

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

int processTask(Task current, int funcId, double *acceptedValue, Task *left, Task *right) {
    double a = current.a;
    double b = current.b;
    double m = 0.5 * (a + b);
    double s = simpsonEstimate(funcId, a, b);
    double sLeft = simpsonEstimate(funcId, a, m);
    double sRight = simpsonEstimate(funcId, m, b);
    double s2 = sLeft + sRight;
    double err = fabs(s2 - s);

    if (err <= 15.0 * current.tol || current.depth >= MAX_DEPTH || fabs(b - a) <= MIN_WIDTH) {
        *acceptedValue = s2 + (s2 - s) / 15.0;
        return 1;
    }

    left->a = a;
    left->b = m;
    left->tol = current.tol / 2.0;
    left->depth = current.depth + 1;
    right->a = m;
    right->b = b;
    right->tol = current.tol / 2.0;
    right->depth = current.depth + 1;
    return 0;
}

double adaptiveSimpson(int funcId, double a, double b, double tol, int depth, long long *acceptedTotal) {
    Task current;
    current.a = a;
    current.b = b;
    current.tol = tol;
    current.depth = depth;
    double acceptedValue = 0.0;
    Task left, right;
    if (processTask(current, funcId, &acceptedValue, &left, &right)) {
        (*acceptedTotal)++;
        return acceptedValue;
    }
    return adaptiveSimpson(funcId, left.a, left.b, left.tol, left.depth, acceptedTotal) + adaptiveSimpson(funcId, right.a, right.b, right.tol, right.depth, acceptedTotal);
}
int chooseStaticK(int numProcs) {
    int k = 8 * numProcs;
    if (k < 32) {
        k = 32;
    }
    return k;
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
    double result = 0.0;
    long long acceptedTotal = 0;

    int chosenK = 0;
    long long localAccepted = 0;
    long long minLocal = 0;
    long long maxLocal = 0;

    if (mode == 0) {
        if (myRank == 0) {
            result = adaptiveSimpson(funcId, 0.0, 1.0, tol, 0, &acceptedTotal);
        }
    }
    else if (mode == 1) {
        chosenK = chooseStaticK(numProcs);

        int base = chosenK / numProcs;
        int rem = chosenK % numProcs;
        int myCount = base + (myRank < rem ? 1 : 0);
        int myStart = myRank * base + (myRank < rem ? myRank : rem);
        double localSum = 0.0;
        localAccepted = 0;
        double localTol = tol / (double)chosenK;

        for (int i = 0; i < myCount; i++) {
            int idx = myStart + i;

            double a = (double)idx / (double)chosenK;
            double b = (double)(idx + 1) / (double)chosenK;

            localSum += adaptiveSimpson(funcId, a, b, localTol, 0, &localAccepted);
        }

        MPI_Reduce(&localSum, &result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&localAccepted, &acceptedTotal, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&localAccepted, &minLocal, 1, MPI_LONG_LONG, MPI_MIN, 0, MPI_COMM_WORLD);
        MPI_Reduce(&localAccepted, &maxLocal, 1, MPI_LONG_LONG, MPI_MAX, 0, MPI_COMM_WORLD);
    }
    else if (mode == 2) {
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();
    if (myRank == 0) {
        printf("funcId = %d\n", funcId);
        printf("mode = %d\n", mode);
        printf("tol = %.12g\n", tol);
        printf("integral = %.15f\n", result);
        printf("acceptedIntervals = %lld\n", acceptedTotal);

        if (mode == 1) {
            double loadRatio = 0.0;
            if (minLocal > 0) {
                loadRatio = (double)maxLocal / (double)minLocal;
            }

            printf("chosenK = %d\n", chosenK);
            printf("localAcceptedMin = %lld\n", minLocal);
            printf("localAcceptedMax = %lld\n", maxLocal);
            printf("loadImbalanceRatio = %.6f\n", loadRatio);
        }
        printf("runtimeSeconds = %.6f\n", end - start);
    }
    MPI_Finalize();
    return 0;
}