#include <stdio.h>
#include <string.h>
#include <mpi.h>

#define K 10000
#define TAG_RANGE 100

int pingPong(int argc, char *argv[], int msg_size){
    int rank, size, tag;
    double start, time;
    char *buffer = malloc(msg_size);
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if(rank==0){
        if(size!=2){
            printf("Only use two processors\n");
            return 1;
        }
        start = MPI_Wtime();
    }
    for(int i = 0; i<K; i++){

        tag = K%TAG_RANGE;
            
        if(rank==0){
            MPI_Send(&buffer, 1, MPI_CHAR, 1, tag, MPI_COMM_WORLD);
            MPI_Recv(&buffer, 1, MPI_CHAR, 1, tag, MPI_COMM_WORLD, NULL);
        }
        if(rank==1){
            MPI_Send(&buffer, 1, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
            MPI_Recv(&buffer, 1, MPI_CHAR, 0, tag, MPI_COMM_WORLD, NULL);
        }
    }
    if(rank==0){
        time = MPI_Wtime() - start;
        printf("Msg Size: %d: Average Round Trip Time is: %f seconds\n", msg_size, (time/K));
        printf("Msg Size: %d: Estimated One Way Latency is: %f seconds\n", msg_size, (time/K)/2);
    }
    MPI_Finalize();
    return 0;
}

int ring(int argc, char *argv[]){
    int rank, size, buffer;
    int send = 10;
    int recv = 20;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Sendrecv(&rank, 1, MPI_INT, (rank+1)%size, send, &buffer, 1, MPI_INT, (rank-1)%size, recv, MPI_COMM_WORLD, NULL);

    printf("Rank %d: sent %d to right, received %d from left\n", rank, rank, buffer);

    MPI_Finalize();
    return 0;
}

int main(int argc, char *argv[]){
    if(strcmp(argv[1], "pingpong")==0){
        pingPong(argc, argv, 1);
        pingPong(argc, argv, 8);
        pingPong(argc, argv, 64);
        pingPong(argc, argv, 512);
        pingPong(argc, argv, 4096);
    }
    else if(strcmp(argv[1], "ring")==0){
        ring(argc, argv);
    }
    else{
        printf("Invalid Input\n");
    }
    return 0;
}