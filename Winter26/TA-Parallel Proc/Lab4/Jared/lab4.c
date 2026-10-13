#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define TAG_RANGE 1000
#define TAG 200
#define K 10000
#define NUM_SIZES 5

void pingpong(int rank);
void ring(int rank, int size);

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (strcmp(argv[1], "pingpong") == 0) {
        if (size != 2) {
            if (rank == 0) {
                printf("Ping-pong requires -np 2\n");
            }
        } else {
            pingpong(rank);
        }
    } else if (strcmp(argv[1], "ring") == 0) {
        if (size < 2) {
            if (rank == 0) {
                printf("Ring requires -np >= 2\n");
            }
        } else {
            ring(rank, size);
        }
    } 

    MPI_Finalize();
    return 0;
}

void pingpong(int rank) {
    int sizes[NUM_SIZES] = {1, 8, 64, 512, 4096};
    if (rank == 0) {
        printf("Message Size (bytes) | Iterations | Avg RTT (us) | Est. One-way (us)\n");
    }

    for (int s = 0; s < NUM_SIZES; s++) {
        int msg_size = sizes[s];
        char *buffer = malloc(msg_size);

        if (buffer == NULL) {
            printf("malloc failed\n");
            return;
        }
        double t0 = 0.0, t1 = 0.0;

        if (rank == 0) {
            t0 = MPI_Wtime();
            for (int k = 0; k < K; k++) {
                int tag = k % TAG_RANGE;
                MPI_Status status;

                MPI_Send(buffer, msg_size, MPI_BYTE, 1, tag, MPI_COMM_WORLD);
                MPI_Recv(buffer, msg_size, MPI_BYTE, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                if (status.MPI_TAG != tag) {
                    printf("Tag error at %d -> expected %d got %d\n", k, tag, status.MPI_TAG);
                }
            }

            t1 = MPI_Wtime();
            double total = t1 - t0;
            double avg_rtt_us = (total / K) * 1e6;
            double one_way_us = avg_rtt_us / 2.0;

            printf("%20d | %10d | %12.3f | %17.3f\n",
                   msg_size, K, avg_rtt_us, one_way_us);
        } else if (rank == 1) {
            for (int k = 0; k < K; k++) {
                int tag = k % TAG_RANGE;
                MPI_Status status;

                MPI_Recv(buffer, msg_size, MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

                if (status.MPI_TAG != tag) {
                    printf("Tag error at %d -> expected %d got %d\n", k, tag, status.MPI_TAG);
                }

                MPI_Send(buffer, msg_size, MPI_BYTE, 0, tag, MPI_COMM_WORLD);
            }
        }

        free(buffer);
    }
}

void ring(int rank, int size) {
    int right = (rank + 1) % size;
    int left  = (rank - 1 + size) % size;
    int send_value = rank;
    int recv_value = -1;
    MPI_Status status;

    MPI_Sendrecv(&send_value, 1, MPI_INT, right, TAG, &recv_value, 1, MPI_INT, left, TAG, MPI_COMM_WORLD, &status);

    printf("Rank %d: sent %d to rank %d, received %d from rank %d\n", rank, send_value, right, recv_value, left);
}