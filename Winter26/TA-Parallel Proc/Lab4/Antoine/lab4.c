#include <stdio.h>
#include <stdlib.h>
#include<mpi.h>
#include <string.h>


#define K 10000

#define TAG_RANGE 1000

// this can be overwritten at compile time
#ifndef MSG_SIZE
#define MSG_SIZE 1
#endif

void pingpong(int size, int rank);
void ring(int size, int rank);

int main(int argc, char **argv){
    if (argc < 2){
        fprintf(stderr, "Incorrect usage, provide a mode, ie :pingpong or ring\n");
        exit(EXIT_FAILURE);
    }

    MPI_Init(&argc, &argv);

    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0){
        printf("MSG_SIZE=%d\n", MSG_SIZE);
    }

    if (strcmp(argv[1], "pingpong") == 0){
        pingpong(size, rank);
    }
    else if (strcmp(argv[1], "ring") == 0){
        ring(size, rank);

    }
    else if (rank == 0){
        fprintf(stderr, "Please select a valid mode\n");
    }


    MPI_Finalize();
    return 0;
}


void pingpong(int size, int rank){
    if (rank == 0){
        char *buffer = malloc(sizeof(char)*MSG_SIZE);
        long double t0 = MPI_Wtime();
        for (int i=0; i<K;i++){

            memcpy(buffer, &i, sizeof(int));

            int tag = i % TAG_RANGE;
            MPI_Send(buffer, MSG_SIZE, MPI_BYTE, 1, tag, MPI_COMM_WORLD);

            MPI_Status status;
            MPI_Recv(buffer, MSG_SIZE, MPI_BYTE, 1, tag, MPI_COMM_WORLD, &status);

        }
        long double t1 = MPI_Wtime();
        long double total_time = t1 - t0;

        double avg_rtt = total_time/K;
        printf("Avg RTT %.10f ms\n", avg_rtt);
        printf("Estimated one-way latency: %.10f ms\n", avg_rtt/2);
        free(buffer);
    }
    else if (rank == 1){
        char *buffer = malloc(sizeof(char)*MSG_SIZE);
        for (int i=0; i<K;i++){
            int tag = i % TAG_RANGE;

            MPI_Status status;
            MPI_Recv(buffer, MSG_SIZE, MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG != tag) {
                printf("Tag mismatch at iteration %d\n", K);
            }

            MPI_Send(buffer, MSG_SIZE, MPI_BYTE, 0, status.MPI_TAG, MPI_COMM_WORLD);
        }
        free(buffer);
    }
}


void ring(int size, int rank){
    int right = (rank + 1) % size;
    int left  = (rank - 1 + size) % size;

    int send_val = rank;
    int recv_val;

    MPI_Sendrecv(&send_val, 1, MPI_INT, right, 0, &recv_val, 1, MPI_INT, left, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf("Rank %d: sent %d to right, received %d from left\n", rank, right, left);
}
