#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG_RANGE 1000
#define K_ITERATIONS 10000
#define TAG01 100
#define TAG10 101
#define TAGR 200

void run_pingpong(int rank) {
    int msg_sizes[] = {1, 8, 64, 512, 4096, 65536};
    int num_sizes = 6;

    if (rank == 0) {
        printf("%-20s | %-12s | %-15s | %-15s\n", 
               "Message Size (bytes)", "Iterations", "Avg RTT (s)", "Est. Latency (s)");
        printf("----------------------------------------------------------------------------\n");
    }

    for (int s = 0; s < num_sizes; s++) {
        int msg_size = msg_sizes[s];
        char *buffer = (char *)malloc(msg_size); 
        
        double start_time = 0.0;
        if (rank == 0) {
            start_time = MPI_Wtime(); 
        }

        for (int k = 0; k < K_ITERATIONS; k++) {
            int tag = k % TAG_RANGE; 

            if (rank == 0) {
                // Rank 0: Send -> Receive 
                MPI_Send(buffer, msg_size, MPI_BYTE, 1, tag, MPI_COMM_WORLD);
                MPI_Recv(buffer, msg_size, MPI_BYTE, 1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else if (rank == 1) {
                // Rank 1: Receive -> Send 
                MPI_Status status;
                MPI_Recv(buffer, msg_size, MPI_BYTE, 0, tag, MPI_COMM_WORLD, &status);
                
                // Verify correctness using tags, not content 
                if (status.MPI_TAG != tag) {
                    printf("Tag mismatch at iteration %d\n", k);
                }
                MPI_Send(buffer, msg_size, MPI_BYTE, 0, tag, MPI_COMM_WORLD);
            }
        }

        if (rank == 0) {
            double end_time = MPI_Wtime();
            double total_time = end_time - start_time;
            double avg_rtt = total_time / K_ITERATIONS; 
            double latency = avg_rtt / 2.0; 

            printf("%-20d | %-12d | %-15.9f | %-15.9f\n", 
                   msg_size, K_ITERATIONS, avg_rtt, latency);
        }
        free(buffer);
    }
}

void run_ring(int rank, int size) {
    int send_value = rank; // Initial value is the rank 
    int recv_value = -1;
    int right = (rank + 1) % size; // Right neighbour logic
    int left = (rank - 1 + size) % size; // Left neighbour logic 

    // Use MPI_Sendrecv to avoid deadlock 
    MPI_Sendrecv(&send_value, 1, MPI_INT, right, TAGR,
                 &recv_value, 1, MPI_INT, left, TAGR,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Confirm the exchange 
    printf("Rank %d: sent %d to right, received %d from left\n", 
           rank, send_value, recv_value);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv); 

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size); 

    if (argc < 2) {
        if (rank == 0) printf("Usage: %s pingpong|ring\n", argv[0]);
        MPI_Finalize();
        return 0;
    }

    if (strcmp(argv[1], "pingpong") == 0) {
        if (size != 2) {
            if (rank == 0) printf("Ping-pong requires -np 2\n"); 
        } else {
            run_pingpong(rank); 
        }
    } 
    else if (strcmp(argv[1], "ring") == 0) {
        if (size < 2) {
            if (rank == 0) printf("Ring requires -np >= 2\n"); 
        } else {
            run_ring(rank, size); 
        }
    }

    MPI_Finalize(); 
    return 0;
}