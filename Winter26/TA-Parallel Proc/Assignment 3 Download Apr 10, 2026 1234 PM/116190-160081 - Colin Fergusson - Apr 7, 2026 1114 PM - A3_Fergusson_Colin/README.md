### Compile Command

mpicc -fopenmp A3.c -lm -o A3

### Example Run Commands

OMP_NUM_THREADS=4 mpirun -np 4 A3 1 2 1e-10
OMP_NUM_THREADS=2 mpirun -np 2 A3 2 0 1e-9
OMP_NUM_THREADS=8 mpirun -np 8 A3 0 1 1e-9

### Machine Specifications

RAM: 16GB DDR4 (Only 8 used to run on VM)
CPU: Intel(R) Core(TM) i7-10750H CPU @ 2.60 (Run using 8 threads out of 12 on a VM)
GPU: NVIDIA GeForce RTX 2060

### MPI Implementation

OpenMPI