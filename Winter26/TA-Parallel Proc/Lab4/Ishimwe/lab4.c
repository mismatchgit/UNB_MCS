#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>


void pingpong( int rank){

    //iterations 
    const int iterations= 10000;

    //message sizes bites
    int sizes[5]= {1, 8,64, 512, 4096};

    // Loop through the message bites 
    for (int s =0 ; s < 5 ; s++ )
	{
        int message_size = sizes[s];
        //message to send (buffer)
        char *buffer = malloc(message_size);
        //MPi status
        MPI_Status status;
        //time capture 
        double start, end;

        if (rank == 0)
            start = MPI_Wtime(); 

        for (int k =0 ; k < iterations; k++)
		{
            //calculate the tag 
            int tag = k % 1000;

            //Make a ping pong where one send and another receive to avoid deadlock

            if ( rank == 0)
			{
                MPI_Send(buffer,message_size, MPI_BYTE,1,tag,MPI_COMM_WORLD);
                MPI_Recv(buffer, message_size,MPI_BYTE,1,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
                //check if the tag matches
                if(status.MPI_TAG != tag){
                    printf("Rank 0: Tag mismatch at iteration %d\n", k);
                 }
            }
            else{
                MPI_Recv(buffer, message_size, MPI_BYTE,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
                if(status.MPI_TAG != tag){
                    printf("Rank 1: Tag mismatch at iteration %d\n", k);
                 }
                MPI_Send(buffer, message_size, MPI_BYTE,0,tag,MPI_COMM_WORLD);
            }

    } //end of iterations

    if(rank == 0)
        {
            end = MPI_Wtime();

            double total = end - start;
            double avg = total / iterations;
            double one_way = avg / 2;

            printf("%d bytes , %d iterations , avg_RTT = %f , one-way latency ≈ %f\n",
                   message_size, iterations, avg, one_way);
        }

        free(buffer);
    }
}

void ring (int rank, int size){

    //calculate the right node 
    int right = (rank + 1) % size;
    //calculate the left node
    int left = (rank -1 +size)% size;

    int send_value = rank;
    int recv_value;
    MPI_Status status;
    //send to right, receive from the left
    MPI_Sendrecv(
        &send_value, 1, MPI_INT, right, 200,
        &recv_value, 1, MPI_INT, left, 200,
        MPI_COMM_WORLD, &status
    );

    printf("RANK %d: sent %d to RANK %d, received %d from RANK %d\n",
           rank, send_value, right, recv_value, left);


}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // check if argument exists
    if (argc < 2)
    {   
        //use processor zero to print to avoid multiple prints
        if(rank == 0)
            printf("The command should be : %s pingpong or ring\n", argv[0]);

        MPI_Finalize();
        return 0;
    }

    // mode selection
    if (strcmp(argv[1], "pingpong") == 0)
    {
        //ensures ping pong only used 2 processors
        if(size != 2)
        {
            if(rank == 0)
                printf("Pingpong messaging requires two ranks\n");
        }
        else
        {
            pingpong(rank);
        }
    }

    else if (strcmp(argv[1], "ring") == 0)
    {
        if(size < 2)
        {
            if(rank == 0)
                printf("Ring messaging requires at least two ranks\n");
        }
        else
        {
            ring(rank, size);
        }
    }

    MPI_Finalize();

    return 0;
}