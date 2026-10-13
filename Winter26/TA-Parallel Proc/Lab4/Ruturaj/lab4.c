#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TAG01 100
#define TAG10 101
#define TAGR 200

void run_pingpong(int rank) {
  int K = 10000;
  int sizes[] = {1, 8, 64, 512, 4096, 65536};
  int num_sizes = 6;
  int TAG_RANGE = 1000;

  if (rank == 0) {
        printf("Message Size (bytes) | Iterations | Avg RTT (s) | Est. Latency (s)\n");
        printf("------------------------------------------------------------------\n");
    }

  // Loop over the different message sizes
  for (int s=0; s < num_sizes; s++) {
    // Create a buffer of the appropriate size
    char *buffer = (char *)malloc(sizes[s]);

    // Start timer
    double start_time = 0.0;
    if (rank == 0) {
      start_time = MPI_Wtime();
    }

    // Ping-pong K times
    for (int i=0; i < K; i++) {
      int tag = i % TAG_RANGE; // Cycle through a range of tags to avoid conflicts

      if (rank == 0) {
        // Send message to rank 1 and wait for reply
        MPI_Send(buffer, sizes[s], MPI_BYTE, 1, tag, MPI_COMM_WORLD);
        MPI_Recv(buffer, sizes[s], MPI_BYTE, 1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      } else {
          MPI_Status status;

        // Wait for message from rank 0 and reply
        MPI_Recv(buffer, sizes[s], MPI_BYTE, 0, tag, MPI_COMM_WORLD, &status);
        // Check that the received message has the expected tag
        if (status.MPI_TAG != tag) {
          printf("Tag mismatch at iteration %d\n", i);
        }
        MPI_Send(buffer, sizes[s], MPI_BYTE, 0, tag, MPI_COMM_WORLD);
      }
    }

    // Stop timer and calculate latency
    if (rank == 0) {
      double end_time = MPI_Wtime();
      double total_time = end_time - start_time;
      double avg_rtt = total_time / K; // Average round-trip time
      double latency = avg_rtt / 2; // One-way latency

      printf("%-20d | %-10d | %-11.9f | %-11.9f\n", 
                   sizes[s], K, avg_rtt, latency);
      fflush(stdout);
    }
    free(buffer);
  }
}
void run_ring(int rank, int size) {
  int send_val = rank; // Value to send, in this case - rank of the process
  int recv_val; // Variable to receive the value from the previous process
  int right = (rank + 1) % size; // Rank of the next process in the ring
  int left = (rank - 1 + size) % size; // Rank of the previous process in the ring
  int tag = 200; // Tag for the messages
  MPI_Status status;

  // Send to the right and receive from the left
  MPI_Sendrecv(&send_val, 1, MPI_INT, right, tag,
                &recv_val, 1, MPI_INT, left, tag,
                MPI_COMM_WORLD, &status);

  // Confirming the exchange by printing the received value
  printf("Rank %d: sent %d to right (Rank %d), received %d from left (Rank %d)\n", 
           rank, send_val, right, recv_val, left);

}

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
      if (rank == 0)
        printf("Usage: %s pingpong|ring\n", argv[0]);
      MPI_Finalize();
      return 0;
    }

    if (strcmp(argv[1], "pingpong") == 0)
    {
      if (size != 2)
      {
        if (rank == 0)
          printf("Ping-pong requires -np 2\n");
      }
      else
      {
        run_pingpong(rank);
      }
    }
    else if (strcmp(argv[1], "ring") == 0)
    {
      if (size < 2)
      {
        if (rank == 0)
          printf("Ring requires -np >= 2\n");
      }
      else
      {
        run_ring(rank, size);
      }
    }

    MPI_Finalize();

    return 0;
}