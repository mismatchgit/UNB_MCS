#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 4000
double* alloc_matrix(int n) {
 double *m = (double*) malloc((size_t)n * (size_t)n * sizeof(double));
 if (!m) {
 printf("Allocation failed!\n");
 exit(1);
 }
 return m;
}
void fill_matrix(double *m, int n, double val) {
 for (int i = 0; i < n*n; i++) m[i] = val;
}
int main(int argc, char **argv) {
 // Optional: run like ./matrix_lab 6
 if (argc >= 2) {
 int t = atoi(argv[1]);
 if (t > 0) omp_set_num_threads(t);
 }
 double *A = alloc_matrix(N);
 double *B = alloc_matrix(N);
 double *C = alloc_matrix(N);
 fill_matrix(A, N, 1.0);
 fill_matrix(B, N, 1.0);
 fill_matrix(C, N, 0.0);
 // =====================================================
 // LEVEL 1: SERIAL BASELINE
 // =====================================================
 // TODO: Implement Level 1 here
 
 // start time
 double start;
 double end;

 start = omp_get_wtime() ;

 

 for (int i=0;i<N; i++){
    for (int j=0; j<N; j++){

        C[i * N + j] =A[i * N + j]+ B[i * N + j];
        

    }
    //printf("%d\t", C[i * N+j]);
 }
    //printf("%d\t", Ci * N+j);
 end= omp_get_wtime() ;


 double total= end-start;

printf("The result is %f \n", C[(N-1) * N + (N-1)]);
printf("The serial took %f seconds\n", total);
printf("----------------------------------------------------------------\n");
 

 
 // =====================================================
 // LEVEL 2: MANUAL BLOCK DECOMPOSITION
 // =====================================================
 // TODO: Implement Level 2A here

 double start_parallel;

 start_parallel= omp_get_wtime() ;
//To set the nimber of threads to 6 for testing and reset the value to not keep the one from the serial
//omp_set_num_threads(6);
C[(N-1) * N + (N-1)]=0.0;
printf("The value for  C[N-1][N-1] before start of level 2A is reseted to = %f\n", C[(N-1) * N + (N-1)]);

 #pragma omp parallel
{
    int id   = omp_get_thread_num();
    int nthr = omp_get_num_threads();

    int chunk = N / nthr;
    int start_row = id * chunk;
    int end_row= start_row+ chunk;
 

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < end_row; j++) {
            C[i * N + j] = A[i * N + j] + B[i * N + j];
        }
    }
}

double end_parallel= omp_get_wtime() ;

printf("The value for  C[N-1][N-1] in parallel is = %f\n", C[(N-1) * N + (N-1)]);

printf("The parallel took %f seconds\n", end_parallel - start_parallel);

printf("----------------------------------------------------------------\n");

 // TODO: Implement Level 2C here
 // =====================================================

//To set the nimber of threads to 6 for testing and reset the value to not keep the one from the serial
//omp_set_num_threads(6);
C[(N-1) * N + (N-1)]=0.0;
 

 double start_parallel_c= omp_get_wtime() ;

 #pragma omp parallel
{
    int id   = omp_get_thread_num();
    int nthr = omp_get_num_threads();

    int chunk = N / nthr;
    int start_row = id * chunk;
    int end_row= start_row+ chunk;
   
    //Make an if statement that takes the last thread and give it to reach the last number
    if (id == nthr - 1){
      end_row= N;
    }

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++ ) {
            C[i * N + j] = A[i * N + j] + B[i * N + j];
        }
    }
}

double end_parallel_c= omp_get_wtime() ;

printf("The value for  C[N-1][N-1] in parallel_C is = %f\n", C[(N-1) * N + (N-1)]);

printf("The Time Level 2 C took is %f seconds\n", end_parallel_c - start_parallel_c);
printf("----------------------------------------------------------------\n");

 // LEVEL 3: CYCLIC DECOMPOSITION
 // =====================================================
 // TODO: Implement Level 3 here
omp_set_num_threads(6);
double start_parallel_lev3= omp_get_wtime() ;
C[(N-1) * N + (N-1)]=0.0;
printf("The value for  C[N-1][N-1] before start of level 3 is reseted to = %f\n", C[(N-1) * N + (N-1)]);

 #pragma omp parallel
{
    int id   = omp_get_thread_num();
    int nthr = omp_get_num_threads();

   //Make a loop that Jumps the nthr
    for (int i = id; i < N ; i += nthr) {
        for (int j = 0; j<N; j++ ) {
            C[i * N + j] = A[i * N + j] + B[i * N + j];
        }
    }
}

double end_parallel_lev3= omp_get_wtime() ;

printf("The value for  C[N-1][N-1] for level 3 is = %f\n", C[(N-1) * N + (N-1)]);

printf("The Time Level 3  took is %f seconds\n", end_parallel_lev3 - start_parallel_lev3);

printf("----------------------------------------------------------------\n");
 // =====================================================
 // LEVEL 4: DEPENDENCY TRAP
 // =====================================================
 // TODO: Implement Level 4A here
   
   //Re-initialize C:
    fill_matrix(C, N, 1.0);
   //C[i][j] = C[i][j] + C[i-1][j]
   //initialize the staring value 
   int a=1;
    for (int i=a;i<N; i++){
      for (int j=0; j<N; j++){

        C[i * N + j] =C[i * N + j]+C[(i-1) * N + j];
        
      }   
   }

   printf("Check Point on Level 4A C[1000][0] is = %f\n", C[(1000) * N + (0)]);


   printf("----------------------------------------------------------------\n");

 // TODO: Implement Level 4B here
 // =====================================================
   //Re-initialize C:
   fill_matrix(C, N, 1.0);
   omp_set_num_threads(4);
   #pragma omp parallel
   {
      int id   = omp_get_thread_num();
      int nthr = omp_get_num_threads();


      int chunk = N / nthr;
      int start_row = id * chunk;
      int end_row= start_row+ chunk;
   
      if (id==0){
         start_row=1;
      }
      for (int i=start_row;i<end_row; i++){
         for (int j=0; j<N; j++){

         C[i * N + j] =C[i * N + j]+C[(i-1) * N + j];
         
         }   
      }


      
   }
   printf("Check Point on Level 4B C[1000][0] is = %f\n", C[(1000) * N + (0)]);

   printf("----------------------------------------------------------------\n");

 // LEVEL 5: BARRIER SYNCHRONIZATION
 // =====================================================
 // TODO: Implement Level 5A here
    //Re-initialize C:
   fill_matrix(C, N, 1.0);
   omp_set_num_threads(4);

   #pragma omp parallel
   {
      int id   = omp_get_thread_num();
      int nthr = omp_get_num_threads();
      
      int chunk = N / nthr;
      int start_row = id * chunk;
      int end_row= start_row+ chunk;
   
      if (id==0){
         start_row=1;
      }
      for (int i=start_row;i<end_row; i++){
         for (int j=0; j<N; j++){

         C[i * N + j] =C[i * N + j]+C[(i-1) * N + j];
         
         }   
      }
      printf("Thread %d finished calculation\n", id);
      printf("Thread %d starting verification\n", id);

      
   }
   
   printf("----------------------------------------------------------------\n");

 // TODO: Implement Level 5B here
   
   fill_matrix(C, N, 1.0);
   omp_set_num_threads(4);

   #pragma omp parallel
   {
      int id   = omp_get_thread_num();
      int nthr = omp_get_num_threads();
      
      int chunk = N / nthr;
      int start_row = id * chunk;
      int end_row= start_row+ chunk;
   
      if (id==0){
         start_row=1;
      }
      for (int i=start_row;i<end_row; i++){
         for (int j=0; j<N; j++){

         C[i * N + j] =C[i * N + j]+C[(i-1) * N + j];
         
         }   
      }
      printf("Thread %d finished calculation\n", id);
      #pragma omp barrier //add a barrier to first wait

      printf("Thread %d starting verification\n", id);

      
   }


 free(A);
 free(B);
 free(C);
 return 0;
}