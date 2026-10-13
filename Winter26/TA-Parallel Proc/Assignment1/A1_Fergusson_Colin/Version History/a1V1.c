#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

int main(int argc, char **argv){
    if(argc != 3){
        printf("Please include the size of the matrices and the mode as command line arguments\n");
        return 1;
    }
    int N = atoi(argv[1]);
    char* mode = argv[2];

    double* A = (double*)malloc(sizeof(double)*N*N);
    double* B = (double*)malloc(sizeof(double)*N*N);

    for(int i = 0; i<N; i++){
        for(int j = 0; j<N; j++){
            A[i*N+j] = sin(i) * cos(j) + sqrt(i+j+1);
            B[i*N+j] = cos(i) * sin(j) + sqrt(i+j+2);
        }
    }
    printf("Generated successfully\n");
    switch(mode[0]){
        case '0':
            break;
        case '1':
            break;
        case '2':
            break;
        case '3':
            if(mode[1]=='A'){

            }
            else if(mode[1]=='B'){
                
            }
            else{
                printf("Invalid Mode\n");
                free(A);
                free(B);
                return 1;
            }
            break;
        case '4':
            break;
        case '5':
            break;
        case '6':
            break;
        default:
            printf("Invalid Mode\n");
            free(A);
            free(B);
            return 1;
    }

    free(A);
    free(B);
}