#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG_RANGE 1000

void pingpong(int rank) {
    const int sizes[] = {1, 8, 64, 512, 4096};
    const int num_sizes = 5;
    const int K = 10000;

    for (int s = 0; s < num_sizes; s++) {
        int msg_size = sizes[s];
        char *buffer = (char*)malloc(msg_size);

        for (int i = 0; i < msg_size; i++) buffer[i] = 0;

        double start = 0.0, end = 0.0;
        if (rank == 0) start = MPI_Wtime();

        for (int k = 0; k < K; k++) {
            int tag = k % TAG_RANGE;

            if (rank == 0) {
                MPI_Send(buffer, msg_size, MPI_BYTE, 1, tag, MPI_COMM_WORLD);
                MPI_Recv(buffer, msg_size, MPI_BYTE, 1, MPI_ANY_TAG,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                MPI_Status status;
                MPI_Recv(buffer, msg_size, MPI_BYTE, 0, MPI_ANY_TAG,
                         MPI_COMM_WORLD, &status);

                if (status.MPI_TAG != tag) {
                    printf("Tag mismatch at iteration %d\n", k);
                }

                MPI_Send(buffer, msg_size, MPI_BYTE, 0, tag, MPI_COMM_WORLD);
            }
        }

        if (rank == 0) {
            end = MPI_Wtime();
            double avg_rtt = (end - start) / K;
            printf("Size %5d bytes | Avg RTT = %.6e s\n", msg_size, avg_rtt);
        }

        free(buffer);
    }
}

void ring(int rank, int size) {
    int right = (rank + 1) % size;
    int left  = (rank - 1 + size) % size;

    int send_value = rank;
    int recv_value = -1;

    MPI_Sendrecv(&send_value, 1, MPI_INT, right, 0,
                 &recv_value, 1, MPI_INT, left,  0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf("Rank %d: sent %d to %d, received %d from %d\n",
           rank, send_value, right, recv_value, left);
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) printf("Usage: ./lab1 pingpong|ring\n");
        MPI_Finalize();
        return 0;
    }

    if (strcmp(argv[1], "pingpong") == 0) {
        if (size == 2) pingpong(rank);
    } else if (strcmp(argv[1], "ring") == 0) {
        if (size >= 2) ring(rank, size);
    }

    MPI_Finalize();
    return 0;
}