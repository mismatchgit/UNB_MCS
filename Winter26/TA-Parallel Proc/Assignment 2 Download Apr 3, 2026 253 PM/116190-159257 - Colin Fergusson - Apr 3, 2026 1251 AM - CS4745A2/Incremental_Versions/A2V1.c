#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

int func0(int mode, int tol, int rank, int size){
    switch(mode){
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        default:
            if(rank==0){
                printf("Invalid mode");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
    }
}

int func1(int mode, int tol, int rank, int size){
    switch(mode){
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        default:
            if(rank==0){
                printf("Invalid mode");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
    }
}

int func2(int mode, int tol, int rank, int size){
    switch(mode){
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        default:
            if(rank==0){
                printf("Invalid mode");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
    }
}

int main(int argc, char** argv){
    MPI_Init(argc, argv);
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(rank==0){
        if(argc!=4){
            printf("Please provide the function id, execution mode, and error tolerance as command line arguments.");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    int func_id = argv[1];
    int mode = argv[2];
    int tol = argv[3];

    switch(func_id){
        case 0:
            func0(mode, tol, rank, size);
            break;
        case 1:
            func1(mode, tol, rank, size);
            break;
        case 2:
            func2(mode, tol, rank, size);
            break;
        default:
            if(rank==0){
                printf("Invalid function id");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
    }

    MPI_Finalize();
    return 0;
}