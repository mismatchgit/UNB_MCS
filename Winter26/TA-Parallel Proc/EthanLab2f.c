#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 4000

double *alloc_matrix(int n) {
  double *m = (double *)malloc((size_t)n * (size_t)n * sizeof(double));
  if (!m) {
    printf("Allocation failed!\n");
    exit(1);
  }
  return m;
}

void fill_matrix(double *m, int n, double val) {
  for (int i = 0; i < n * n; i++)
    m[i] = val;
}
int static get_index(int i, int j) { return (i * N) + j; }

void matrix_debug(double *matrix) {
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      printf("%f ", matrix[get_index(i, j)]);
    }
    printf("\n");
  }
  printf("\n");
}

void s2A(double *matrix, double *A, double *B) {

  double dawn = omp_get_wtime();
#pragma omp parallel
  {
    int id = omp_get_thread_num();
    int T = omp_get_num_threads();

    int chunk = N / T;
    int start = id * chunk;
    int end = start + chunk;

    for (int i = start; i < end; ++i) {
      for (int j = 0; j < N; ++j) {
        matrix[get_index(i, j)] = A[get_index(i, j)] + B[get_index(i, j)];
      }
    }
  }

  double dusk = omp_get_wtime();
  printf("L2A: Elapsed: %f\n", dusk - dawn);
}
void s2C(double *matrix, double *A, double *B) {

  double dawn = omp_get_wtime();
  int T = 0;
#pragma omp parallel
  {
    int id = omp_get_thread_num();
    T = omp_get_num_threads();

    int chunk = N / T;
    int start = id * chunk;
    int end = start + chunk;

    for (int i = start; i < end; ++i) {
      for (int j = 0; j < N; ++j) {
        matrix[get_index(i, j)] = A[get_index(i, j)] + B[get_index(i, j)];
      }
    }
  }

  int rem = N - (N % T);

  for (int i = rem + 1; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      matrix[get_index(i, j)] = A[get_index(i, j)] + B[get_index(i, j)];
    }
  }

  double dusk = omp_get_wtime();
  printf("L2C: Elapsed: %f\n", dusk - dawn);
}

void s3(double *matrix, double *A, double *B) {

  int tcount = omp_get_max_threads();
  int round = 0;
  double dawn = omp_get_wtime();
  while (round <= (N / tcount)) {
#pragma omp parallel
    {
      int id = omp_get_thread_num();
      int index = (round * tcount) + id;
      // PERF: we can remove this guard if add another pragma block for the
      // remainding elements
      if (index < N) {
        for (int i = 0; i < N; ++i) {
          matrix[get_index(index, i)] =
              A[get_index(index, i)] + B[get_index(index, i)];
        }
      }
    }
    round++;
  }
  double dusk = omp_get_wtime();
  printf("L3: Elapsed: %f\n", dusk - dawn);
}

void s4A(double *matrix) {
  for (int i = 1; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      matrix[get_index(i, j)] =
          matrix[get_index(i, j)] + matrix[get_index(i - 1, j)];
    }
  }

  printf("L4A: C[1000][0] = %f\n", matrix[1000 * N]);
}

void s4B(double *matrix) {
  int max = omp_get_max_threads();

  omp_set_num_threads(4);
#pragma omp parallel
  {
    int id = omp_get_thread_num();
    int chunk = N / 4;
    int start = id * chunk + 1;
    int end = id * chunk + chunk;

    for (int i = start; i < end; ++i) {
      for (int j = 0; j < N; ++j) {
        matrix[get_index(i, j)] =
            matrix[get_index(i, j)] + matrix[get_index(i - 1, j)];
      }
    }
  }

  int rem = N - (N % 4);

  for (int i = rem + 1; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      matrix[get_index(i, j)] =
          matrix[get_index(i, j)] + matrix[get_index(i - 1, j)];
    }
  }

  printf("L4B: C[1000][0] = %f\n", matrix[1000 * N]);
  omp_set_num_threads(max);
}

void s5A(double *matrix) {
  int max = omp_get_max_threads();

  omp_set_num_threads(4);
#pragma omp parallel
  {
    int id = omp_get_thread_num();
    int chunk = N / 4;
    int start = id * chunk + 1;
    int end = id * chunk + chunk;

    for (int i = start; i < end; ++i) {
      for (int j = 0; j < N; ++j) {
        matrix[get_index(i, j)] =
            matrix[get_index(i, j)] + matrix[get_index(i - 1, j)];
      }
    }
    printf("Thread %d finished calculation\n", id);
    printf("Thread %d starting verification\n", id);
  }

  int rem = N - (N % 4);

  for (int i = rem + 1; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      matrix[get_index(i, j)] =
          matrix[get_index(i, j)] + matrix[get_index(i - 1, j)];
    }
  }

  printf("L4B: C[1000][0] = %f\n", matrix[1000 * N]);
  omp_set_num_threads(max);
}

void s5B(double *matrix) {
  int max = omp_get_max_threads();

  omp_set_num_threads(4);
#pragma omp parallel
  {
    int id = omp_get_thread_num();
    int chunk = N / 4;
    int start = id * chunk + 1;
    int end = id * chunk + chunk;

    for (int i = start; i < end; ++i) {
      for (int j = 0; j < N; ++j) {
        matrix[get_index(i, j)] =
            matrix[get_index(i, j)] + matrix[get_index(i - 1, j)];
      }
    }
    printf("Thread %d finished calculation\n", id);
#pragma omp barrier
    printf("Thread %d starting verification\n", id);
  }

  int rem = N - (N % 4);

  for (int i = rem + 1; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      matrix[get_index(i, j)] =
          matrix[get_index(i, j)] + matrix[get_index(i - 1, j)];
    }
  }

  printf("L4B: C[1000][0] = %f\n", matrix[1000 * N]);
  omp_set_num_threads(max);
}

int main(int argc, char **argv) {

  // Optional: run like ./matrix_lab 6
  if (argc >= 2) {
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

  double dawn = omp_get_wtime();
  for (int i = 0; i < N * N; ++i) {
    C[i] = A[i] + B[i];
  }
  double dusk = omp_get_wtime();
  printf("L1: Elapsed: %f\n", dusk - dawn);

  // =====================================================
  // LEVEL 2: MANUAL BLOCK DECOMPOSITION
  // =====================================================

  fill_matrix(C, N, 0.0);
  s2A(C, A, B);
  printf("C[N-1][N-1] = %f\n", C[(N * N)]);

  fill_matrix(C, N, 0.0);
  s2C(C, A, B);

  printf("C[N-1][N-1] = %f\n", C[(N * N) - 1]);

  // =====================================================
  // LEVEL 3: CYCLIC DECOMPOSITION
  // =====================================================

  fill_matrix(C, N, 0.0);
  s3(C, A, B);
  printf("C[N-1][N-1] = %f\n", C[(N * N) - 1]);

  // =====================================================
  // LEVEL 4: DEPENDENCY TRAP
  // =====================================================

  fill_matrix(C, N, 1.0);
  s4A(C);

  // TODO: Implement Level 4B here
  fill_matrix(C, N, 1.0);
  s4B(C);

  // =====================================================
  // LEVEL 5: BARRIER SYNCHRONIZATION
  // =====================================================

  // TODO: Implement Level 5A here
  fill_matrix(C, N, 1.0);
  s5A(C);

  fill_matrix(C, N, 1.0);
  s5B(C);

  // TODO: Implement Level 5B here

  free(A);
  free(B);
  free(C);
  return 0;
}
