#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

double functions(double x, int func_id, int rank){
    switch(func_id){
        case 0:
            return sin(x)+0.5*cos(3*x);
            break;
        case 1:
            return 1/(1+100*pow(x-0.3,2));
            break;
        case 2:
            return sin(200*x)*exp(x-(2*x));
            break;
        default:
            if(rank==0){
                printf("Invalid function id");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
    }
}


int mode0(int func_id, int tol, double a, double b, int rank, int size){
    double m = (a+b)/2;
}

int mode1(int func_id, int tol, double a, double b, int rank, int size){
    double m = (a+b)/2;
}

int mode2(int func_id, int tol, double a, double b, int rank, int size){
    double m = (a+b)/2;
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

    double a = 0;
    double b = 1;

    switch(mode){
        case 0:
            mode0(func_id, tol, a, b, rank, size);
            break;
        case 1:
            mode1(func_id, tol, a, b, rank, size);
            break;
        case 2:
            mode2(func_id, tol, a, b, rank, size);
            break;
        default:
            if(rank==0){
                printf("Invalid mode");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
    }

    MPI_Finalize();
    return 0;
}