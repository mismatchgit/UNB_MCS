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
    if(rank==0){
        double m = (a+b)/2;
        double simpson = ((b-a)/2)*(functions(a, func_id, rank) + 4*functions(m, func_id, rank) + functions(b, func_id, rank));

        double leftm = (a+m)/2;
        double rightm = (m+b)/2;

        double leftSimpson = ((m-a)/2)*(functions(a, func_id, rank) + 4*functions(leftm, func_id, rank) + functions(m, func_id, rank));
        double rightSimpson = ((b-m)/2)*(functions(m, func_id, rank) + 4*functions(rightm, func_id, rank) + functions(b, func_id, rank));

        if(fabs((leftSimpson+rightSimpson)-simpson)<15*tol){
            return leftSimpson+rightSimpson+((leftSimpson+rightSimpson)-simpson)/15;
        }

        return mode0(func_id, tol/2, a, m, rank, size) + mode0(func_id, tol/2, m, b, rank, size);
    }
}

int mode1(int func_id, int tol, double a, double b, int rank, int size){
    double m = (a+b)/2;
}

int mode2(int func_id, int tol, double a, double b, int rank, int size){
    double m = (a+b)/2;
}

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(rank==0){
        if(argc!=4){
            printf("Please provide the function id, execution mode, and error tolerance as command line arguments.");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    int func_id = atoi(argv[1]);
    int mode = atoi(argv[2]);
    double tol = atof(argv[3]);

    double a = 0;
    double b = 1;
    double result;

    switch(mode){
        case 0:
            result = mode0(func_id, tol, a, b, rank, size);
            break;
        case 1:
            result = mode1(func_id, tol, a, b, rank, size);
            break;
        case 2:
            result = mode2(func_id, tol, a, b, rank, size);
            break;
        default:
            if(rank==0){
                printf("Invalid mode");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
    }

    printf("Function %d, outputted %f", func_id, result);

    MPI_Finalize();
    return 0;
}