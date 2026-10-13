#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 4000

double *alloc_matrix(int n)
{
	double *m = (double *)malloc((size_t)n * (size_t)n * sizeof(double));
	if (!m)
	{
		printf("Allocation failed!\n");
		exit(1);
	}
	return m;
}

void fill_matrix(double *m, int n, double val)
{
	for (int i = 0; i < n * n; i++)
		m[i] = val;
}

int main(int argc, char **argv)
{

	// Optional: run like ./matrix_lab 6
	if (argc >= 2)
	{
		int t = atoi(argv[1]);
		if (t > 0)
			omp_set_num_threads(t);
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

	double start, end;

	start = omp_get_wtime();

	for (int i = 0; i < N; i++)
	{
		for (int j = i; j < N; j++)
		{
			C[(i * N) + j] = A[(i * N) + j] + B[(i * N) + j];
		}
	}

	end = omp_get_wtime();

	printf("\n////////////////////////////////////\n");
	printf("Serial \nTime : %.10f \nLast Element : %.2f", (end - start), C[(N * N) - 1]);
	printf("\n////////////////////////////////////\n");

	// =====================================================
	// LEVEL 2: MANUAL BLOCK DECOMPOSITION
	// =====================================================

	// TODO: Implement Level 2A here

	fill_matrix(A, N, 1.0);
	fill_matrix(B, N, 1.0);
	fill_matrix(C, N, 0.0);

	start = omp_get_wtime();

#pragma omp parallel
	{
		int threads = omp_get_num_threads();
		int id = omp_get_thread_num();

		int chunk = N / threads;
		int start = id * chunk;
		int end = start + chunk;

		for (int i = start; i < end; i++)
		{
			for (int j = i; j < end; j++)
			{
				C[(i * end) + j] = A[(i * end) + j] + B[(i * end) + j];
			}
		}
	}

	end = omp_get_wtime();

	printf("\n////////////////////////////////////\n");
	printf("Manual Block \nTime : %.10f \nLast Element : %.2f", (end - start), C[(N * N) - 1]);
	printf("\n////////////////////////////////////\n");

	// TODO: Implement Level 2C here

	fill_matrix(A, N, 1.0);
	fill_matrix(B, N, 1.0);
	fill_matrix(C, N, 0.0);

	start = omp_get_wtime();

#pragma omp parallel
	{
		int threads = omp_get_num_threads();
		int id = omp_get_thread_num();

		int chunk = N / threads;
		int start = id * chunk;
		int end = start + chunk;

		if (id == threads - 1)
		{
			end = N;
		}

		for (int i = start; i < end; i++)
		{
			for (int j = i; j < end; j++)
			{
				C[(i * end) + j] = A[(i * end) + j] + B[(i * end) + j];
			}
		}
	}

	end = omp_get_wtime();

	printf("\n////////////////////////////////////\n");
	printf("Manual Block (Bug Fix) \nTime : %.10f \nLast Element : %.2f", (end - start), C[(N * N) - 1]);
	printf("\n////////////////////////////////////\n");

	// =====================================================
	// LEVEL 3: CYCLIC DECOMPOSITION
	// =====================================================

	// TODO: Implement Level 3 here

	fill_matrix(A, N, 1.0);
	fill_matrix(B, N, 1.0);
	fill_matrix(C, N, 0.0);

	start = omp_get_wtime();

#pragma omp parallel
	{
		int threads = omp_get_num_threads();
		int id = omp_get_thread_num();

		for (int i = id; i < (N * N); i += threads)
		{
			C[i] = A[i] + B[i];
		}
	}

	end = omp_get_wtime();

	printf("\n////////////////////////////////////\n");
	printf("Cyclic Decomposition \nTime : %.10f \nLast Element : %.2f", (end - start), C[(N * N) - 1]);
	printf("\n////////////////////////////////////\n");

	// =====================================================
	// LEVEL 4: DEPENDENCY TRAP
	// =====================================================

	// TODO: Implement Level 4A here

	fill_matrix(C, N, 1.0);

	start = omp_get_wtime();

	for (int i = 1; i < N; i++)
	{
		for (int j = i; j < N; j++)
		{
			C[(i * N) + j] = C[(i * N) + j] + C[((i - 1) * N) + j];
		}
	}

	end = omp_get_wtime();

	printf("\n////////////////////////////////////\n");
	printf("Serial Dependency \nTime : %.10f \nCheckpoint Element : %.2f", (end - start), C[(5623)]);
	printf("\n////////////////////////////////////\n");

	// TODO: Implement Level 4B here
	fill_matrix(C, N, 1.0);

	start = omp_get_wtime();

#pragma omp parallel
	{
		int threads = omp_get_num_threads();
		int id = omp_get_thread_num();

		int chunk = N / threads;
		int start = id * chunk;
		int end = start + chunk;

		for (int i = start; i < end; i++)
		{
			for (int j = i; j < end; j++)
			{
				C[(i * start) + j] = C[(i * start) + j] + C[((i - 1) * start) + j];
			}
		}
	}

	end = omp_get_wtime();

	printf("\n////////////////////////////////////\n");
	printf("Manual Block Dependency \nTime : %.10f \nCheckpoint Element : %.2f", (end - start), C[(5623)]);
	printf("\n////////////////////////////////////\n");

	// -->

	// =====================================================
	// LEVEL 5: BARRIER SYNCHRONIZATION
	// =====================================================

	// TODO: Implement Level 5A here
	printf("\n////////////////////////////////////\nSynchronization (Part A)\n");
	start = omp_get_wtime();

#pragma omp parallel
	{
		int threads = omp_get_num_threads();
		int id = omp_get_thread_num();

		int chunk = N / threads;
		int start = id * chunk;
		int end = start + chunk;

		if (id == threads - 1)
		{
			end = N;
		}

		for (int i = start; i < end; i++)
		{
			for (int j = i; j < end; j++)
			{
				C[(i * end) + j] = A[(i * end) + j] + B[(i * end) + j];
			}
		}

		printf("\nThread %d finished calculation\n", id);
		printf("Thread %d starting verification\n", id);
	}

	end = omp_get_wtime();

	printf("\nTime : %.10f", (end - start));
	printf("\n////////////////////////////////////\n");

	/*
	Answer:
	Some threads print start verification before other threads complete their calcultion
	*/

	// TODO: Implement Level 5B here

	printf("\n////////////////////////////////////\nSynchronization (Part B)\n");
	start = omp_get_wtime();

#pragma omp parallel
	{
		int threads = omp_get_num_threads();
		int id = omp_get_thread_num();

		int chunk = N / threads;
		int start = id * chunk;
		int end = start + chunk;

		if (id == threads - 1)
		{
			end = N;
		}

		for (int i = start; i < end; i++)
		{
			for (int j = i; j < end; j++)
			{
				C[(i * end) + j] = A[(i * end) + j] + B[(i * end) + j];
			}
		}

		printf("\nThread %d finished calculation\n", id);

#pragma omp barrier

		printf("Thread %d starting verification\n", id);
	}

	end = omp_get_wtime();

	printf("\nTime : %.10f", (end - start));
	printf("\n////////////////////////////////////\n");

	/*
	Answer:
	No thread prints start verification before other threads complete their calcultion
	*/

	free(A);
	free(B);
	free(C);
	return 0;
}
