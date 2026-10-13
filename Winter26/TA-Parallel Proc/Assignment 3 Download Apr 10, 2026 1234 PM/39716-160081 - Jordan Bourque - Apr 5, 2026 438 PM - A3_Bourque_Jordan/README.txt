Version History

a3v0 - Copied and pasted from Assignment 2.
Timestamp: April 5, 12:30pm.

a3v1 - Changed Mode 2 to be the new Mode 1, as the assignment dictated. Attempted to implement omp reduction, but kept running into syntax errors and improper formatting.
Timestamp: April 5, 2:00pm.

a3v2 - Had multiple errors regarding not getting proper values between same-function runs. Turns out I had a simple sum = -> sum += error, but took some experimentation to realize this simple mistake. This final version was used as integration.c.
Timestamp: April 5, 2:45pm.

Compile command: gcc -O3 -fopenmp integration.c -o integration -I"C:\Program Files (x86)\Microsoft SDKs\MPI\Include" -L"C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64" -lmsmpi -lm

NOTE: This compile command is used because my environment variables aren't set correctly, or something (my other ones do - my computer just doesn't like MPI). Your compile command will likely be different, especially if you are using Linux.

Example typical compile command: mpicc -fopenmp integration.c -lm -o integration

Example run commands: Windows - $env:OMP_NUM_THREADS=2
				mpiexec -n 4 ./integration 4 0 2 1e-8
                      Linux   - OMP_NUM_THREADS=2
				mpirun -np 4 ./integration 4 0 2 1e-6

Machine specifications are included in the performance report file.

MPI implementation used: Microsoft MPI Startup Program [Version 10.1.12498.52]