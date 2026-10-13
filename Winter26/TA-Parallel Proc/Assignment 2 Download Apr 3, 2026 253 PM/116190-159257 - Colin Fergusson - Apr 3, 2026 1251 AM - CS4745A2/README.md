### Compile Command

mpicc A2.c -o A2 -lm

### Example Run Commands

mpirun -np 4 A2 1 2 1e-8
mpirun -np 8 A2 0 0 1e-6
mpirun -np 2 A2 2 1 1e-6

### Machine Specifications

RAM: 16GB DDR4 (Only 8 used to run on VM)
CPU: Intel(R) Core(TM) i7-10750H CPU @ 2.60 (Run using 8 threads out of 12 on a VM)
GPU: NVIDIA GeForce RTX 2060

### MPI Implementation

OpenMPI