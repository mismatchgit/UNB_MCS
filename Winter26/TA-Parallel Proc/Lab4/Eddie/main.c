#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define TAG_RANGE 1000

void pingpong(const int rank)
{
    size_t sizes[] = {1, 8, 64, 512, 4096};
    size_t max_iter = 10000, msg_size = 1;
    if (rank == 0)
        printf("\nMessage Size(bytes) | Iterations(k) | Avg RTT(µs) | Est.One - way(µs)\n");

    for (int i = 0; i < 5; i++)
    {
        msg_size = sizes[i];
        char *buffer = malloc(msg_size * sizeof(char));
        if (buffer == NULL)
            return;
        double t0 = MPI_Wtime();
        for (size_t k = 0; k < max_iter; k++)
        {
            int tag = k % TAG_RANGE;

            if (rank == 0)
            {
                MPI_Send(buffer, msg_size, MPI_BYTE, 1, tag, MPI_COMM_WORLD);

                MPI_Status status;
                MPI_Recv(buffer, msg_size, MPI_BYTE, 1, MPI_ANY_TAG,
                         MPI_COMM_WORLD, &status);
                if (status.MPI_TAG != tag)
                {
                    printf("Tag mismatch at iteration %ld\n", k);
                }
            }
            else
            {
                MPI_Status status;
                MPI_Recv(buffer, msg_size, MPI_BYTE, 0, MPI_ANY_TAG,
                         MPI_COMM_WORLD, &status);
                if (status.MPI_TAG != tag)
                {
                    printf("Tag mismatch at iteration %ld\n", k);
                }

                MPI_Send(buffer, msg_size, MPI_BYTE, 0, tag, MPI_COMM_WORLD);
            }
        }
        if (rank == 0)
        {
            double t1 = MPI_Wtime();
            double total_time = t1 - t0;
            double average_RTT = (total_time / max_iter) * 1e6; // Convert to microseconds
            double latency = average_RTT / 2;
            printf(" %li \t| %li \t| %.5f \t| %.5f \n", msg_size, max_iter, average_RTT, latency);
        }
        free(buffer);
    }
}

void ring(int rank, int size)
{
    int right = (rank + 1) % size;
    int left = (rank - 1 + size) % size;

    int send_val = rank;
    int recv_val = -1;
    MPI_Status status;

    MPI_Sendrecv(&send_val, 1, MPI_INT, right, 200,
                 &recv_val, 1, MPI_INT, left, 200,
                 MPI_COMM_WORLD, &status);

    printf("Rank %i: sent %i to rank %i, received %i from rank %i\n", rank, send_val, right, recv_val, left);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2)
    {
        if (rank == 0)
            printf("Usage: mpirun -np {processors} ./main {command}\n");
        MPI_Finalize();
        return 1;
    }

    char *cmd = argv[1];

    if (strcmp(cmd, "pingpong") == 0)
    {
        if (size != 2)
        {
            if (rank == 0)
                printf("Ping-pong requires -np 2\n");
        }
        else
        {
            pingpong(rank);
        }
    }
    else if (strcmp(cmd, "ring") == 0)
    {
        if (size < 2)
        {
            if (rank == 0)
                printf("Ring requires -np >= 2\n");
        }
        else
        {
            ring(rank, size);
        }
    }

    MPI_Finalize();
    return 0;
}