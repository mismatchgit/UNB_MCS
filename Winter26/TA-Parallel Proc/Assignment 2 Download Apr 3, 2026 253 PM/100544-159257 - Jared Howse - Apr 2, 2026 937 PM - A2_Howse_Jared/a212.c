#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_DEPTH 60
#define MIN_WIDTH 1e-15

#define TAG_REQUEST 100
#define TAG_TASK    101
#define TAG_RESULT  102
#define TAG_SPLIT   103
#define TAG_STOP    104

typedef struct {
    double a;
    double b;
    double tol;
    int depth;
} Task;

typedef struct {
    double partial;
    long long accepted;
} ResultMsg;

typedef struct {
    Task *data;
    int head;
    int size;
    int cap;
} TaskQueue;

void queueInit(TaskQueue *q, int cap) {
    q->cap = (cap > 0) ? cap : 16;
    q->data = (Task *)malloc((size_t)q->cap * sizeof(Task));
    q->head = 0;
    q->size = 0;
}

void queueDestroy(TaskQueue *q) {
    free(q->data);
    q->data = NULL;
    q->head = 0;
    q->size = 0;
    q->cap = 0;
}

void queueGrow(TaskQueue *q) {
    int newCap = q->cap * 2;
    Task *newData = (Task *)malloc((size_t)newCap * sizeof(Task));

    for (int i = 0; i < q->size; i++) {
        newData[i] = q->data[(q->head + i) % q->cap];
    }

    free(q->data);
    q->data = newData;
    q->cap = newCap;
    q->head = 0;
}

void queuePush(TaskQueue *q, Task t) {
    if (q->size == q->cap) {
        queueGrow(q);
    }

    int tail = (q->head + q->size) % q->cap;
    q->data[tail] = t;
    q->size++;
}

Task queuePop(TaskQueue *q) {
    Task t = q->data[q->head];
    q->head = (q->head + 1) % q->cap;
    q->size--;
    return t;
}

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

int chooseStaticK(int numProcs, int funcId) {
    int k = 8 * numProcs;

    if (k < 32) {
        k = 32;
    }

    if (funcId == 2 && k < 512) {
        k = 512;
    }

    return k;
}

void dispatchStandbyWorkers(TaskQueue *queue, int *standbyList, int *standbyCount, int *isStandby, int *busyWorkers) {
    while (queue->size > 0 && *standbyCount > 0) {
        int worker = standbyList[--(*standbyCount)];
        isStandby[worker] = 0;

        Task t = queuePop(queue);
        MPI_Send(&t, sizeof(Task), MPI_BYTE, worker, TAG_TASK, MPI_COMM_WORLD);
        (*busyWorkers)++;
    }
}

void runDynamicQueue(int funcId, double tol, int myRank, int numProcs, double *result, long long *acceptedTotal) {
    if (numProcs == 1) {
        if (myRank == 0) {
            *result = adaptiveSimpson(funcId, 0.0, 1.0, tol, 0, acceptedTotal);
        }
        return;
    }

    if (myRank == 0) {
        TaskQueue queue;
        queueInit(&queue, 64);

        Task root;
        root.a = 0.0;
        root.b = 1.0;
        root.tol = tol;
        root.depth = 0;
        queuePush(&queue, root);

        int *standbyList = (int *)malloc((size_t)numProcs * sizeof(int));
        int *isStandby = (int *)calloc((size_t)numProcs, sizeof(int));
        int *stopped = (int *)calloc((size_t)numProcs, sizeof(int));

        int standbyCount = 0;
        int busyWorkers = 0;
        int stoppedWorkers = 0;
        double globalSum = 0.0;
        long long totalAcc = 0;

        while (stoppedWorkers < numProcs - 1) {
            if (queue.size == 0 && busyWorkers == 0) {
                while (standbyCount > 0) {
                    int worker = standbyList[--standbyCount];
                    if (!stopped[worker]) {
                        int dummy = 0;
                        MPI_Send(&dummy, 1, MPI_INT, worker, TAG_STOP, MPI_COMM_WORLD);
                        stopped[worker] = 1;
                        isStandby[worker] = 0;
                        stoppedWorkers++;
                    }
                }

                if (stoppedWorkers == numProcs - 1) {
                    break;
                }
            }

            MPI_Status status;
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int src = status.MPI_SOURCE;

            if (status.MPI_TAG == TAG_REQUEST) {
                int dummy = 0;
                MPI_Recv(&dummy, 1, MPI_INT, src, TAG_REQUEST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (queue.size > 0) {
                    Task t = queuePop(&queue);
                    MPI_Send(&t, sizeof(Task), MPI_BYTE, src, TAG_TASK, MPI_COMM_WORLD);
                    busyWorkers++;
                }
                else if (busyWorkers == 0) {
                    int stopDummy = 0;
                    MPI_Send(&stopDummy, 1, MPI_INT, src, TAG_STOP, MPI_COMM_WORLD);
                    if (!stopped[src]) {
                        stopped[src] = 1;
                        stoppedWorkers++;
                    }
                }
                else if (!isStandby[src]) {
                    standbyList[standbyCount++] = src;
                    isStandby[src] = 1;
                }
            }
            else if (status.MPI_TAG == TAG_RESULT) {
                ResultMsg msg;
                MPI_Recv(&msg, sizeof(ResultMsg), MPI_BYTE, src, TAG_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                globalSum += msg.partial;
                totalAcc += msg.accepted;
                busyWorkers--;
                dispatchStandbyWorkers(&queue, standbyList, &standbyCount, isStandby, &busyWorkers);
            }
            else if (status.MPI_TAG == TAG_SPLIT) {
                Task spawned;
                MPI_Recv(&spawned, sizeof(Task), MPI_BYTE, src, TAG_SPLIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                queuePush(&queue, spawned);
                dispatchStandbyWorkers(&queue, standbyList, &standbyCount, isStandby, &busyWorkers);
            }
        }

        free(standbyList);
        free(isStandby);
        free(stopped);
        queueDestroy(&queue);

        *result = globalSum;
        *acceptedTotal = totalAcc;
    }
    else {
        while (1) {
            int dummy = 1;
            MPI_Send(&dummy, 1, MPI_INT, 0, TAG_REQUEST, MPI_COMM_WORLD);

            MPI_Status status;
            MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == TAG_STOP) {
                int stopDummy = 0;
                MPI_Recv(&stopDummy, 1, MPI_INT, 0, TAG_STOP, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                break;
            }
            else if (status.MPI_TAG == TAG_TASK) {
                Task current;
                MPI_Recv(&current, sizeof(Task), MPI_BYTE, 0, TAG_TASK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                while (1) {
                    double part = 0.0;
                    Task left, right;

                    if (processTask(current, funcId, &part, &left, &right)) {
                        ResultMsg msg;
                        msg.partial = part;
                        msg.accepted = 1;
                        MPI_Send(&msg, sizeof(ResultMsg), MPI_BYTE, 0, TAG_RESULT, MPI_COMM_WORLD);
                        break;
                    }
                    else {
                        MPI_Send(&right, sizeof(Task), MPI_BYTE, 0, TAG_SPLIT, MPI_COMM_WORLD);
                        current = left;
                    }
                }
            }
        }
    }
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
        chosenK = chooseStaticK(numProcs, funcId);

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
        runDynamicQueue(funcId, tol, myRank, numProcs, &result, &acceptedTotal);
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

        if (mode == 2 && numProcs == 1) {
            printf("dynamicNote = singleProcessFallbackToSerial\n");
        }

        printf("runtimeSeconds = %.6f\n", end - start);
    }

    MPI_Finalize();
    return 0;
}