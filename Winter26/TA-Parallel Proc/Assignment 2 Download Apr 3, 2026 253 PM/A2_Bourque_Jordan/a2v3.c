#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double runTimes[11];
int K = 10;

double function0(double x)
{
    return (sin(x) + (0.5 * cos(3 * x)));
}

double function1(double x)
{
    return (1 / (1 + (100 * ((x - 0.3) * (x - 0.3)))));
}

double function2(double x)
{
    return sin(200 * x) * exp(-x);
}

double simpsonApprox(int func_id, double a, double b)
{
    double m = (a + b) / 2.0;

    double functionA;
    double functionM;
    double functionC;

    if(func_id == 0)
    {
        functionA = function0(a);
        functionM = function0(m);
        functionC = function0(b);
    }
    else if(func_id == 1)
    {
        functionA = function1(a);
        functionM = function1(m);
        functionC = function1(b);
    }
    else
    {
        functionA = function2(a);
        functionM = function2(m);
        functionC = function2(b);
    }

    return ((b - a) / 6.0) * (functionA + 4.0 * functionM + functionC);
}

double serialBaseline(int func_id, double a, double b, double tol, int rank)
{
    if(rank == 0)
    {
        double m = (a + b) / 2.0;

        double S  = simpsonApprox(func_id, a, b);
        double S1 = simpsonApprox(func_id, a, m);
        double S2 = simpsonApprox(func_id, m, b);

        if ((S1 + S2 - S < tol) && (S1 + S2 - S > -tol))
        {
            return S1 + S2;
        }

        return serialBaseline(func_id, a, m, tol/2.0, rank) + serialBaseline(func_id, m, b, tol/2.0, rank);
    }

    return 0;
}

double staticSched(int func_id, double tol, int rank, int size)
{
    double split = (1.0 / K);
    double sum = 0.0;

    for(int i = rank; i < K; i += size)
    {
        double a = i * split;
        double b = (i + 1) * split;
        sum += serialBaseline(func_id, a, b, tol, 0);
    }

    double totalSum = 0.0;

    MPI_Reduce(&sum, &totalSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    return totalSum;
}

double dynamicSched(int func_id, double tol, int rank, int size)
{
    double result = 0.0;

    if(rank == 0)
    {
        double queue[10000][2];
        int front = 0;
        int back = 0;

        queue[back][0] = 0.0;
        queue[back][1] = 1.0;
        back++;

        int activeWorkers = 0;

        while(front < back || activeWorkers > 0)
        {
            MPI_Status status;
            double message[3];

            MPI_Recv(message, 3, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            int src = status.MPI_SOURCE;

            if(status.MPI_TAG == 0)
            {
                if(front < back)
                {
                    double sendData[2];
                    sendData[0] = queue[front][0];
                    sendData[1] = queue[front][1];
                    front++;

                    MPI_Send(sendData, 2, MPI_DOUBLE, src, 1, MPI_COMM_WORLD);
                    activeWorkers++;
                }
                else
                {
                    MPI_Send(NULL, 0, MPI_DOUBLE, src, 2, MPI_COMM_WORLD);
                }
            }
            else if(status.MPI_TAG == 1)
            {
                result += message[0];
                activeWorkers--;
            }
            else if(status.MPI_TAG == 2)
            {
                queue[back][0] = message[0];
                queue[back][1] = message[1];
                back++;
                activeWorkers--;
            }
        }
    }
    else
    {
        while(1)
        {
            MPI_Send(NULL, 0, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

            double recvData[2];
            MPI_Status status;

            MPI_Recv(recvData, 2, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if(status.MPI_TAG == 2)
            {
                break;
            }

            double a = recvData[0];
            double b = recvData[1];

            double m = (a + b) / 2.0;

            double S  = simpsonApprox(func_id, a, b);
            double S1 = simpsonApprox(func_id, a, m);
            double S2 = simpsonApprox(func_id, m, b);

            if((S1 + S2 - S < tol) && (S1 + S2 - S > -tol))
            {
                double sendData[1];
                sendData[0] = S1 + S2;

                MPI_Send(sendData, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
            }
            else
            {
                double sendData[2];
                sendData[0] = m;
                sendData[1] = b;

                MPI_Send(sendData, 2, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);

                double newInterval[2];
                newInterval[0] = a;
                newInterval[1] = m;

                MPI_Send(newInterval, 2, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
            }
        }
    }

    MPI_Bcast(&result, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    return result;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 5)
    {
        if(rank == 0)
        {
            printf("\nPlease enter P, func_id, mode, and tol.");
        }
        MPI_Finalize();
        return 1;
    }

    int P           = atoi(argv[1]);
    int func_id     = atoi(argv[2]);
    int mode        = atoi(argv[3]);
    double tol      = atof(argv[4]);

    for(int i = 0; i < 11; i++)
    {
        MPI_Barrier(MPI_COMM_WORLD);
        double start = MPI_Wtime();

        double result = 0;

        if(mode == 0)
        {
            if(rank == 0)
            {
                result = serialBaseline(func_id, 0.0, 1.0, tol, rank);
            }
        }
        if(mode == 1)
        {
            result = staticSched(func_id, tol, rank, size);
        }        
        if(mode == 2)
        {
            result = dynamicSched(func_id, tol, rank, size);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        double end = MPI_Wtime();

        if(rank == 0)
        {
            double totalTime = end - start;
            runTimes[i] = totalTime;
            printf("\nRun %d Mode %d, Result %.10f, Time %.9f \t tolerance: %.9f", i, mode, result, totalTime, tol);
            fflush(stdout);
        }
    }

    if(rank == 0)
    {
        double absoluteTime = 0.0;

        for (int i = 1; i < 11; i++)
        {
            absoluteTime += runTimes[i];
        }

        double avg = absoluteTime / 10.0;

        printf("\n------------\nMode %d, Average Processing time taken (discarding run 0): %.9f\n------------", mode, avg);
    }

    MPI_Finalize();
    return 0;
}