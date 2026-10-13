Compile:
mpicc -O3 integration.c -lm -o integration

Run:
mpirun -np P ./integration funcId mode tol

Arguments:
funcId = 0, 1, or 2
mode = 0, 1, or 2
tol = positive tolerance such as 1e-6 or 1e-8

Modes:
mode 0 = serial baseline
mode 1 = static MPI
mode 2 = dynamic MPI

Examples:
mpirun -np 1 ./integration 1 0 1e-6
mpirun -np 4 ./integration 1 1 1e-6
mpirun -np 4 ./integration 1 2 1e-6
mpirun -np 1 ./integration 2 0 1e-8
mpirun -np 4 ./integration 2 1 1e-8
mpirun -np 4 ./integration 2 2 1e-8

Processor	Intel(R) Core(TM) Ultra 7 258V, 2200 Mhz, 8 Core(s), 8 Logical Processor(s)