1)Compile Command

gcc -O3 -fopenmp a1.c -lm -o a1



2)Example Run Commands

setenv OMP\_NUM\_THREADS 8; ./a1 1024 3



setenv OMP\_NUM\_THREADS 1; ./a1 512 0



setenv OMP\_NUM\_THREADS 16;./a1 2048 5



3)Machine Specifications

Machine: UNB Remote Lab Server



OpenMP version: GCC with -fopenmp



OS: Linux (Remote Lab Environment)

4\) Matrix Sizes Used

512,1024,2048

