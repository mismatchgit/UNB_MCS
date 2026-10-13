
Compile Command
mpicc -fopenmp integration.c -lm -o integration
_______________________________________________________________________________________
Example Run Commands
Mode 1: Dynamic MPI Master/Worker

Run with 4 MPI processes:

mpirun -np 4 ./integration 1 1 1e-6
Mode 2: Hybrid MPI + OpenMP

Run with 2 MPI processes and 4 OpenMP threads per process:

OMP_NUM_THREADS=4 mpirun -np 2 ./integration 1 2 1e-8
Machine Specifications

Processor: AMD Ryzen 3 5300U with Radeon Graphics (2.60 GHz)
RAM: 32 GB
Architecture: 64-bit x86_64
Operating System: Windows 10 (WSL Ubuntu environment)
_________________________________________________________________________________________
MPI Implementation Used

OpenMPI