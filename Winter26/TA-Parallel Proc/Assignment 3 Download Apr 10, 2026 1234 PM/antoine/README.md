## Compile

```bash
make
```
## Examples
```
./serial 0 1e-6
./serial 1 1e-6
./serial 2 1e-6

mpirun -np 8 ./mpi 0 1e-6
mpirun -np 8 ./mpi 1 1e-6
mpirun -np 8 ./mpi 2 1e-6

mpirun -np 8 ./mpi_hybrid 0 1e-6
mpirun -np 8 ./mpi_hybrid 1 1e-6
mpirun -np 8 ./mpi_hybrid 2 1e-6
```


## Machine specifications

OS: Arch Linux (x86\_64)
CPU: Intel Core i7-12700H (14 cores, 20 threads, up to 4.7 GHz)
GPU: NVIDIA RTX 3050 Ti Mobile + Intel Iris Xe
Memory: 32 GB RAM
Kernel: Linux 6.19.10-zen


## MPI Implementation

MPI Library via mpicc and mpirun

