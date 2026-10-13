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
                return 1;
            }
    }
}


double mode0(int func_id, double tol, double a, double b, int rank, int size){
    if(rank==0){
        double m = (a+b)/2;
        double simpson = ((b-a)/6)*(functions(a, func_id, rank) + 4*functions(m, func_id, rank) + functions(b, func_id, rank));

        double leftm = (a+m)/2;
        double rightm = (m+b)/2;

        double leftSimpson = ((m-a)/6)*(functions(a, func_id, rank) + 4*functions(leftm, func_id, rank) + functions(m, func_id, rank));
        double rightSimpson = ((b-m)/6)*(functions(m, func_id, rank) + 4*functions(rightm, func_id, rank) + functions(b, func_id, rank));

        if(fabs((leftSimpson+rightSimpson)-simpson)<15*tol){
            return leftSimpson+rightSimpson+((leftSimpson+rightSimpson)-simpson)/15;
        }

        return mode0(func_id, tol/2, a, m, rank, size) + mode0(func_id, tol/2, m, b, rank, size);
    }
    else{return 0;}
}

double adaptiveSimpson(double a, double b, double tol, int func_id, int rank){
    double m = (a+b)/2;

    double simpson = ((b-a)/6)*(functions(a, func_id, rank) + 4*functions(m, func_id, rank) + functions(b, func_id, rank));

    double leftm = (a+m)/2;
    double rightm = (m+b)/2;

        double leftSimpson = ((m-a)/6)*(functions(a, func_id, rank) + 4*functions(leftm, func_id, rank) + functions(m, func_id, rank));
        double rightSimpson = ((b-m)/6)*(functions(m, func_id, rank) + 4*functions(rightm, func_id, rank) + functions(b, func_id, rank));

        if(fabs((leftSimpson+rightSimpson)-simpson)<15*tol){
            return leftSimpson+rightSimpson+((leftSimpson+rightSimpson)-simpson)/15;
        }

        return adaptiveSimpson(a, m, tol/2, func_id, rank) + adaptiveSimpson(m, b, tol/2, func_id, rank);
}

double mode1(int func_id, double tol, double a, double b, int K, int rank, int size){
    int countTag = 1;
    int indiciesTag = 2;
    int count;
    int* indicies;
    if(rank==0){
        int sections = K/size;
        int remainder = K%size;
        int offset = 0;
        for(int i = 0; i<size; i++){
            count = sections;
            if(i<remainder){
                count++;
            }

            if(i==0){
                indicies = malloc(count*sizeof(int));
                for(int j = 0; j<count; j++){
                    indicies[j] = j;
                }
            }
            else{
                MPI_Send(&count, 1, MPI_INT, i, countTag, MPI_COMM_WORLD);

                int* buffer = malloc(count*sizeof(int));
                for(int j = 0; j<count; j++){
                    buffer[j] = offset+j;
                }

                MPI_Send(buffer, count, MPI_INT, i, indiciesTag, MPI_COMM_WORLD);
                free(buffer);
            }
            offset = offset + count;
        }   
    }
    else{
        MPI_Recv(&count, 1, MPI_INT, 0, countTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        indicies = malloc(count*sizeof(int));
        MPI_Recv(indicies, count, MPI_INT, 0, indiciesTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    double sum = 0;
    for(int i = 0; i<count; i++){
        a = (double)indicies[i]/K;
        b = (double)(indicies[i]+1)/K;

        sum = sum + adaptiveSimpson(a, b, tol/K, func_id, rank);
    }

    double totalSum;
    MPI_Reduce(&sum, &totalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    return totalSum;
}

double mode2(int func_id, double tol, double a, double b, int rank, int size){
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
    double result, start, end, sstart, send;
    double average = 0;
    double saverage = 0;
    int K = 16;

    switch(mode){
        case 0:
            for(int i = 0; i<10; i++){
                if(rank==0){
                    start = MPI_Wtime();
                }
                result = mode0(func_id, tol, a, b, rank, size);
                if(rank==0){
                    end = MPI_Wtime();
                    if(i!=0){average = average + (end-start);}
                }
            }            
            break;
        case 1:
            for(int i = 0; i<10; i++){
                if(rank==0){
                    start = MPI_Wtime();
                }
                result = mode1(func_id, tol, a, b, rank, size, K);
                if(rank==0){
                    end = MPI_Wtime();
                    if(i!=0){average = average + (end-start);}
                }
            }
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
    
    if(rank==0){
        printf("Function %d outputted: %f and took an average of %fs\n", func_id, result, average/9);
    }

    MPI_Finalize();
    return 0;
}