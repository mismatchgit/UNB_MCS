#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

double f(int mode, double x) {
    switch(mode) {
        case 0: {
            return sin(x) + 0.5 * cos(3*x);
        }
        case 1:{
            return 1.0 / (1.0 + 100*(x - 0.3)*(x - 0.3));
        }
        case 2:{
            return sin(200*x) * exp(-x);
        }
        default:{
            return 0.0;
        }
    }
}

inline double simpson(int func, double a, double b) {
    double m = (a + b)/2; // the middle point
    return (b-a)/6 * (f(func,a) + 4*f(func,m) + f(func,b));
}

double adaptive(int func, double a, double b, double tol) {
    double m = (a+b)/2;
    double S = simpson(func, a, b);
    double Sleft = simpson(func, a, m);
    double Sright = simpson(func, m, b);

    if (fabs(Sleft + Sright - S) < 15 * tol) {
        return Sleft + Sright;
    }
    return adaptive(func, a, m, tol/2) + adaptive(func, m, b, tol/2);
}

double serial(const int func, const double tol){
    int K = 64;
    double res = 0.0;
    for (int i = 0; i < K; i++) {
        double a = (double)i / K;
        double b = (double)(i + 1) / K;
        res += adaptive(func, a, b, tol / K);
    }
    return res;
}

#ifndef SERIAL_MODE
double static_mpi(const int func, double tol, const int rank, const int size){
    const int K = 64;

    double local_tol = tol/K;
    double local_sum = 0.0;

    int count = 0;
    for (int i = rank; i < K; i += size) {
        double a = (double)i / K;
        double b = (double)(i + 1) / K;
        local_sum += adaptive(func, a, b, local_tol);
    }

    double global_sum;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    return global_sum;
}
#endif

#ifndef SERIAL_MODE

#define WORK 1
#define RESULT 2
#define STOP 3

typedef struct {
    double a, b, tol;
} Task;

void worker(int func) {
    MPI_Status status;
    while ( true ){
        Task task;
        MPI_Recv(&task, sizeof(Task), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == STOP){
            break;
        }

        double m = (task.a + task.b)/2;
        double Sm = simpson(func, task.a, task.b);
        double Sa = simpson(func, task.a, m);
        double Sb = simpson(func, m, task.b);

        if (fabs(Sa + Sb - Sm) < 15*task.tol ){
            double res = Sa + Sb;
            MPI_Send(&res, 1, MPI_DOUBLE, 0, RESULT, MPI_COMM_WORLD);
        } else {
            Task new_task = { m, task.b, task.tol/2 };
            task.b = m;
            task.tol /= 2;

            MPI_Send(&new_task, sizeof(Task), MPI_BYTE, 0, WORK, MPI_COMM_WORLD);

            double local_result = 0.0;
            int stack = 1;
            Task stack_tasks[100];
            stack_tasks[0] = task;

            while(stack > 0) {
                Task cur = stack_tasks[--stack];
                double m2 = (cur.a + cur.b)/2;
                double S0 = simpson(func, cur.a, cur.b);
                double S1 = simpson(func, cur.a, m2);
                double S2 = simpson(func, m2, cur.b);

                if (fabs(S1 + S2 - S0) < 15*cur.tol) {
                    local_result += S1 + S2;
                } else {
                    stack_tasks[stack++] = (Task){cur.a, m2, cur.tol/2};
                    stack_tasks[stack++] = (Task){m2, cur.b, cur.tol/2};
                }
            }

            MPI_Send(&local_result, 1, MPI_DOUBLE, 0, RESULT, MPI_COMM_WORLD);
        }
    }
}

double master(int func, double tol, int size) {
    int max_queue = 100000;
    Task *queue = malloc(sizeof(Task) * max_queue);
    int front = 0, back = 0;

    int K = 64;
    for (int i = 0; i < K; i++) {
        queue[back++] = (Task){ (double)i/K, (double)(i+1)/K, tol/K };
    }

    double total_result = 0.0;
    int active_tasks = 0;
    int worker_busy[size];
    memset(worker_busy, 0, sizeof(worker_busy));

    while (front < back || active_tasks > 0) {
        for (int i = 1; i < size && front < back; i++) {
            if (!worker_busy[i]) {
                MPI_Send(&queue[front++], sizeof(Task), MPI_BYTE, i, WORK, MPI_COMM_WORLD);
                worker_busy[i] = 1;
                active_tasks++;
            }
        }

        if (active_tasks == 0) {
            break;
        }

        MPI_Status status;
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == RESULT) {
            double partial;
            MPI_Recv(&partial, 1, MPI_DOUBLE, status.MPI_SOURCE, RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_result += partial;
            worker_busy[status.MPI_SOURCE] = 0; // this worker can now receive more work
            active_tasks--;
        }
        else if (status.MPI_TAG == WORK) {
            Task new_task;
            MPI_Recv(&new_task, sizeof(Task), MPI_BYTE, status.MPI_SOURCE, WORK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (back < max_queue) {
                queue[back++] = new_task;
            } else {
                fprintf(stderr, "Queue Overflow! Increase max_queue.\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
    }

    free(queue);

    for (int i = 1; i < size; i++) {
        MPI_Send(NULL, 0, MPI_BYTE, i, STOP, MPI_COMM_WORLD);
    }
    return total_result;
}

#endif

int main(int argc, char **argv){
    if (argc != 4){
        printf("Incorrect usage, Correct usage would be <mode> <func> <tol>");
        exit(EXIT_FAILURE);
    }

    int mode = atoi(argv[1]);
    int func = atoi(argv[2]);
    double tol = atof(argv[3]);

#ifndef SERIAL_MODE
    int rank, size;
    if (mode > 0){
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
    }
#endif

    // if this is not serial then use MPI_Wtime instead
    double start_w, end_w;
    clock_t start_c, end_c;
    if (mode == 0) start_c = clock();
#ifndef SERIAL_MODE
    else start_w = MPI_Wtime();
#endif
    clock_t start = clock();
    double res = 0.0;

    switch (mode){
        case 0:
            res = serial(func, tol);
            break;
#ifndef SERIAL_MODE
        case 1:
            res = static_mpi(func, tol, rank, size);
            break;
        case 2:
            if (rank == 0){
                res = master(func, tol, size);
            } else {
                worker(func);
            }
            break;
#endif
    }

    if (mode == 0) {
        end_c = clock();
        printf("Result: %.10f\nTime: %lf\n", res, (double)(end_c - start_c) / CLOCKS_PER_SEC);
    } else {
#ifndef SERIAL_MODE
        if (rank == 0){
            end_w = MPI_Wtime();
            printf("Result: %.10f\nTime: %lf\n", res, end_w - start_w);
        }
        MPI_Finalize();
#endif
    }
    return 0;
}
