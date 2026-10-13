Compile Command
	Linux/macOS (OpenMPI):
		mpicc integration.c -o integration
Example Run Commands

	Static mode (Mode 1), Function 1, tolerance 1e-6 with 2 processes:

		mpirun -n 2 integration 1 1 1e-6

	Dynamic mode (Mode 2), Function 2, tolerance 1e-8 with 4 processes:

		mpirun --host localhost:4 -np 4 ./integration 2 1 1e-8
Machine Specifications
	RAM: 16 GB
	OS: Windows 10
	Cores: 4

