#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stddef.h>
#include <mpi.h>
#include <omp.h>

typedef struct{
    double a;
    double b;
    double tol;
    int depth;
} Task;

double functions(double x, int func_id){
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
            MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

double mode0(int func_id, double tol, double a, double b, int rank, int size){
    if(rank==0){
        double m = (a+b)/2;
        double simpson = ((b-a)/6)*(functions(a, func_id) + 4*functions(m, func_id) + functions(b, func_id));

        double leftm = (a+m)/2;
        double rightm = (m+b)/2;

        double leftSimpson = ((m-a)/6)*(functions(a, func_id) + 4*functions(leftm, func_id) + functions(m, func_id));
        double rightSimpson = ((b-m)/6)*(functions(m, func_id) + 4*functions(rightm, func_id) + functions(b, func_id));

        if(fabs((leftSimpson+rightSimpson)-simpson)<15*tol){
            return leftSimpson+rightSimpson+((leftSimpson+rightSimpson)-simpson)/15;
        }

        return mode0(func_id, tol/2, a, m, rank, size) + mode0(func_id, tol/2, m, b, rank, size);
    }
    else{return 0;}
}

double mode1(int func_id, double tol, int K, int rank, int size, int* acceptedIntervals){

    //converting Task struct into an MPI Type
    int blockLengths[4] = {1,1,1,1};
    MPI_Aint displacements[4] = {offsetof(Task, a),offsetof(Task, b), offsetof(Task, tol), offsetof(Task, depth)};
    MPI_Datatype types[4] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_INT};
    MPI_Datatype MPI_TASK;

    MPI_Type_create_struct(4, blockLengths, displacements, types, &MPI_TASK);
    MPI_Type_commit(&MPI_TASK);

    const int requestTag = 0;
    const int taskTag = 1;
    const int newTag = 2;
    const int completeTag = 3;
    if(rank==0){
        double sum = 0;
        Task* queue = malloc(K*100*sizeof(Task));
        Task newTask;
        for(int i = 0; i<K; i++){
            newTask.a = (double)i/K;
            newTask.b = (double)(i+1)/K;
            newTask.tol = tol/K;
            newTask.depth = 0;
            queue[i] = newTask; 
        }

        int tasks = K;
        int totalTasks = K;
        int nextTask = 0;
        int workers = size-1;

        while(workers>0){

            MPI_Status status;

            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            int workerRank = status.MPI_SOURCE;
            int tag = status.MPI_TAG;
            
            if(tag==requestTag){
                int garbage;
                MPI_Recv(&garbage, 1, MPI_INT, workerRank, requestTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if(nextTask < totalTasks){
                    MPI_Send(&queue[nextTask], 1, MPI_TASK, workerRank, taskTag, MPI_COMM_WORLD);
                    nextTask++;
                }
                else if(tasks==0){
                    Task terminationTask;
                    terminationTask.a = -1;
                    terminationTask.b = -1;
                    terminationTask.tol = -1;
                    terminationTask.depth = -1;
                    MPI_Send(&terminationTask, 1, MPI_TASK, workerRank, taskTag, MPI_COMM_WORLD);
                    
                    workers--;
                }
                else{
                    Task waitTask;
                    waitTask.a = -2;
                    waitTask.b = -2;
                    waitTask.tol = -2;
                    waitTask.depth = -2;

                    MPI_Send(&waitTask, 1, MPI_TASK, workerRank, taskTag, MPI_COMM_WORLD);
                }
            }
            else if(tag==completeTag){
                double result;
                MPI_Recv(&result, 1, MPI_DOUBLE, workerRank, completeTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                sum=sum+result;
                tasks--;
            }
            else if(tag==newTag){
                Task newTask;
                MPI_Recv(&newTask, 1, MPI_TASK, workerRank, newTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                queue[totalTasks] = newTask;
                tasks++;
                totalTasks++;
            }      
        }
        free(queue);
        *acceptedIntervals = totalTasks;
        return sum;
    }
    else{
        Task terminationTask;
        terminationTask.a = 0;
        terminationTask.b = 0;
        terminationTask.tol = 0;
        terminationTask.depth = 0;

        Task stack[1000];
        int top = -1;
        int wait = 0;

        while(terminationTask.a==0){

            if(top<0){
                MPI_Send(&rank, 1, MPI_INT, 0, requestTag, MPI_COMM_WORLD);

                Task curTask;
                MPI_Recv(&curTask, 1, MPI_TASK, 0, taskTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if(curTask.a==-1){break;}
                if(curTask.a==-2){wait=1;}
                else{wait=0;}

                if(wait==0){
                    top++;
                    stack[top]=curTask;
                }
            }

            if(wait==0){
                Task curTask = stack[top];
                top--;

                double m = (curTask.a+curTask.b)/2;
                double simpson = ((curTask.b-curTask.a)/6)*(functions(curTask.a, func_id) + 4*functions(m, func_id) + functions(curTask.b, func_id));

                double leftm = (curTask.a+m)/2;
                double rightm = (m+curTask.b)/2;

                double leftSimpson = ((m-curTask.a)/6)*(functions(curTask.a, func_id) + 4*functions(leftm, func_id) + functions(m, func_id));
                double rightSimpson = ((curTask.b-m)/6)*(functions(m, func_id) + 4*functions(rightm, func_id) + functions(curTask.b, func_id));

                if(fabs((leftSimpson+rightSimpson)-simpson)<15*curTask.tol){
                    double result = leftSimpson+rightSimpson+((leftSimpson+rightSimpson)-simpson)/15;
                    MPI_Send(&result, 1, MPI_DOUBLE, 0, completeTag, MPI_COMM_WORLD);
                }
                else{
                    top++;
                    Task leftTask;
                    leftTask.a = curTask.a;
                    leftTask.b = m;
                    leftTask.tol = curTask.tol/2;
                    leftTask.depth = curTask.depth+1;
                    stack[top] = leftTask;

                    Task rightTask;
                    rightTask.a = m;
                    rightTask.b = curTask.b;
                    rightTask.tol = curTask.tol/2;
                    rightTask.depth = curTask.depth+1;
                    MPI_Send(&rightTask, 1, MPI_TASK, 0, newTag, MPI_COMM_WORLD);
                }
            }
        }
        return 0;
    }
}

double adaptiveSimpson(double a, double b, double tol, int func_id){
    double m = (a+b)/2;

    double simpson = ((b-a)/6)*(functions(a, func_id) + 4*functions(m, func_id) + functions(b, func_id));

    double leftm = (a+m)/2;
    double rightm = (m+b)/2;

        double leftSimpson = ((m-a)/6)*(functions(a, func_id) + 4*functions(leftm, func_id) + functions(m, func_id));
        double rightSimpson = ((b-m)/6)*(functions(m, func_id) + 4*functions(rightm, func_id) + functions(b, func_id));

        if(fabs((leftSimpson+rightSimpson)-simpson)<15*tol){
            return leftSimpson+rightSimpson+((leftSimpson+rightSimpson)-simpson)/15;
        }

        double left = 0;
        double right = 0;

        #pragma omp task shared(left)
        left = adaptiveSimpson(a, m, tol/2, func_id);

        #pragma omp task shared(right)
        right = adaptiveSimpson(m, b, tol/2, func_id);

        #pragma omp taskwait

        return left+right;
}

double mode2(int func_id, double tol, int K, int rank, int size){
    const int countTag = 0;
    const int indiciesTag = 1;
    int count;
    int* indicies;
    double a,b;
    if(rank==0){
        int* counts = malloc(size*sizeof(int));
        int* offsets = malloc(size*sizeof(int));

        #pragma omp parallel for
        for(int i = 0; i<size; i++){
            if(i<K%size){
                counts[i] = K/size+1;
            }
            else{
                counts[i] = K/size;
            }
        }


        offsets[0]=0;
        #pragma omp parallel for
        for(int i = 1; i<size; i++){
            offsets[i] = offsets[i-1] + counts[i-1];
        }

        int** buffers = malloc(size*sizeof(int*));
        #pragma omp parallel for
        for(int i = 0; i<size; i++){
            buffers[i] = malloc(counts[i]*sizeof(int));
            for(int j = 0; j<counts[i]; j++){
                buffers[i][j] = offsets[i] + j;
            }
        }

        count = counts[0];
        indicies = malloc(count*sizeof(int));
        indicies = buffers[0];

        for(int i = 1; i<size; i++){
            MPI_Send(&counts[i], 1, MPI_INT, i, countTag, MPI_COMM_WORLD);
            MPI_Send(buffers[i], counts[i], MPI_INT, i, indiciesTag, MPI_COMM_WORLD);
        }
        free(counts);
        free(offsets);
        free(buffers);
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
        #pragma omp parallel
        {
            #pragma omp single
            sum = sum + adaptiveSimpson(a, b, tol/K, func_id);
        }   
        
    }

    free(indicies);

    double totalSum;
    MPI_Reduce(&sum, &totalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if(rank==0){return totalSum;}
    else{return 0;}
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

    if(rank==0){
        if(func_id>2||func_id<0){
            printf("Please provide a valid function id");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    double a = 0;
    double b = 1;
    double result, sresult, start, end, sstart, send;
    double average = 0;
    double saverage = 0;
    int K = 99;
    int acceptedIntervals;

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
                result = mode1(func_id, tol, K, rank, size, &acceptedIntervals);
                if(rank==0){
                    end = MPI_Wtime();
                    if(i!=0){average = average + (end-sstart);}

                    sstart = MPI_Wtime();
                }
                sresult = mode0(func_id, tol, a, b, rank, size);
                if(rank==0){
                    send = MPI_Wtime();
                    if(i!=0){saverage = saverage + (send-sstart);}
                }
            }
            break;
        case 2:
            for(int i = 0; i<10; i++){
                if(rank==0){
                    start = MPI_Wtime();
                }
                result = mode2(func_id, tol, K, rank, size);
                if(rank==0){
                    end = MPI_Wtime();
                    if(i!=0){average = average + (end-start);}

                    sstart = MPI_Wtime();
                }
                sresult = mode0(func_id, tol, a, b, rank, size);
                if(rank==0){
                    send = MPI_Wtime();
                    if(i!=0){saverage = saverage + (send-sstart);}
                }
            }
            break;
        default:
            if(rank==0){
                printf("Invalid mode");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
    }
    
    if(rank==0){
        average = average/9;
        printf("Function %d outputted: %0.15f and took an average of %0.15fs\n", func_id, result, average);
        if(mode!=0){
            saverage = saverage/9;
            if(fabs(result-sresult)<=10*tol){
                printf("Parallel output is within tolerance\n");
            }
            else{
                printf("Parallel output is not within tolerance\n");
            }
            printf("Serial time: %0.15fs\n", saverage);
            printf("Speedup is: %0.15f\n", saverage/average);
            printf("Efficiency is: %0.15f\n", (saverage/average)/size);
            printf("K is: %d\n", K);
            if(mode!=1){
                acceptedIntervals=K;
            }
            printf("The total number of accepted intervals is: %d\n", acceptedIntervals);
        }
        
    }

    MPI_Finalize();
    return 0;
}