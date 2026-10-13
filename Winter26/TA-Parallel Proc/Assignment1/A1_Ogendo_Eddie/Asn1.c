#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define MAX(a, b) ((a - b) > (0) ? (a) : (b))

double *allocMatZeros(const int N)
{
    double *c = (double *)calloc(N * N, sizeof(double));
    if (c == NULL)
    {
        printf("Allocation Failed");
        return NULL;
    }

    return c;
}

double *allocMat(const int N, const int val)
{
    double *mat = (double *)malloc(N * N * sizeof(double));
    if (mat == NULL)
        return NULL;

    if (val == 1)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                mat[i * N + j] = sin(i) * cos(j) + sqrt(i + j + val);
            }
        }
    }
    else
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                mat[i * N + j] = cos(i) * sin(j) + sqrt(i + j + val);
            }
        }
    }

    return mat;
}

double *square_mat(double *const a, double *const b, const int N)
{
    double *c = allocMatZeros(N);
    if (c == NULL)
    {
        free(a);
        free(b);
        exit(1);
    }

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            double sum = 0.0;
            for (int k = 0; k < N; k++)
            {
                sum += a[i * N + k] * b[k * N + j];
            }
            c[i * N + j] = sum;
        }
    }
    return c;
}

double *square_m1(double *const a, double *const b, const int N)
{
    double *c = allocMatZeros(N);
    if (c == NULL)
    {
        free(a);
        free(b);
        exit(1);
    }

#pragma omp parallel for shared(c, a, b, N) schedule(dynamic)
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            double sum = 0.0;
            for (int k = 0; k < N; k++)
            {
                sum += a[i * N + k] * b[k * N + j];
            }
            c[i * N + j] = sum;
        }
    }

    return c;
}

double *square_m2(double *const a, double *const b, const int N)
{
    double *c = allocMatZeros(N);
    if (c == NULL)
    {
        free(a);
        free(b);
        exit(1);
    }

#pragma omp parallel for collapse(2) shared(a, b, c, N)
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            double sum = 0.0;
            for (int k = 0; k < N; k++)
            {
                sum += a[i * N + k] * b[k * N + j];
            }
            c[i * N + j] = sum;
        }
    }

    return c;
}

double *square_m4(double *const a, double *const b, const int N)
{
    double *c = allocMatZeros(N);
    if (c == NULL)
    {
        free(a);
        free(b);
        exit(1);
    }

    int block_size = 64;

#pragma omp parallel
    {

#pragma omp single
        {
#pragma omp taskgroup
            {
                for (int row_start = 0; row_start < N; row_start += block_size)
                {
#pragma omp task shared(a, b, c, N) firstprivate(row_start)
                    {
                        int row_end = (row_start + block_size > N) ? N : row_start + block_size;
                        for (int i = row_start; i < row_end; i++)
                        {
                            for (int j = 0; j < N; j++)
                            {
                                double sum = 0;
                                for (int k = 0; k < N; k++)
                                {
                                    sum += a[i * N + k] * b[k * N + j];
                                }
                                c[i * N + j] = sum;
                            }
                        }
                    }
                }
            }
        }
    }

    return c;
}

double *square_m5(double *const a, double *const b, const int N)
{
    double *c = allocMatZeros(N);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            double sum = 0.0;
#pragma omp simd
            for (int k = 0; k < N; k++)
            {
                sum += a[i * N + k] * b[k * N + j];
            }
            c[i * N + j] = sum;
        }
    }
    return c;
}

double *square_m6(double *const a, double *const b, const int N)
{
    double *c = allocMatZeros(N);
#pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            double sum = 0;
#pragma omp simd reduction(+ : sum)
            for (int k = 0; k < N; k++)
            {
                sum += a[i * N + k] * b[k * N + j];
            }
            c[i * N + j] = sum;
        }
    }
    return c;
}

double sum_mat(double *const x, const int N)
{
    double sum = 0;

    for (int row = 0; row < N * N; row++)
    {
        sum += x[row];
    }

    return sum;
}

double sum_mat1(double *const x, const int N)
{
    double sum = 0;
    const int size = N * N;

#pragma omp parallel for default(none) shared(x) firstprivate(size) schedule(static) reduction(+ : sum)
    for (int row = 0; row < size; row++)
    {
        sum += x[row];
    }

    return sum;
}

double sum_mat1b(double *const x, const int N)
{
    double sum = 0;
    const int size = N * N;
    const int chunk = size / omp_get_num_threads();

#pragma omp parallel for default(none) shared(x, chunk) firstprivate(size) schedule(dynamic, chunk) reduction(+ : sum)
    for (int row = 0; row < size; row++)
    {
        sum += x[row];
    }

    return sum;
}

double max_mat(double *const x, const int N)
{
    double maxV = -1e18;
    int size = N * N;

    for (int row = 0; row < size; row++)
    {
        maxV = MAX(x[row], maxV);
    }

    return maxV;
}

double max_mat1(double *const x, const int N)
{
    double maxV = -1e18;
    int size = N * N;

#pragma omp parallel for reduction(max : maxV) shared(x)
    for (int row = 0; row < size; row++)
    {
        maxV = MAX(x[row], maxV);
    }

    return maxV;
}

long long checksum(double *const x, const int N)
{
    long long checksum = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            checksum += (long long)(x[i * N + j] * 1000.0) % 100000;
        }
    }
    return checksum;
}

long long checksum3A(double *const x, const int N)
{
    long long checksum = 0;

#pragma omp parallel for collapse(2) shared(checksum)
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            long long val = (long long)(x[i * N + j] * 1000.0) % 100000;
#pragma omp atomic
            checksum += val;
        }
    }
    return checksum;
}

long long checksum3B(double *const x, const int N)
{
    long long checksum = 0;

#pragma omp parallel for default(none) collapse(2) shared(x, N, checksum)
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            long long val = (long long)(x[i * N + j] * 1000.0) % 100000;
#pragma omp critical
            {
                checksum += val;
            }
        }
    }
    return checksum;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Error!!\nInsufficient input expected: ./{prog} {N} {mode}\n");
        return 1;
    }

    const int N = atoi(argv[1]);
    const int mode = atoi(argv[2]);

    if (mode == 5)
    {
        omp_set_num_threads(1);
    }
    else
    {
        int num_t = omp_get_max_threads();
        omp_set_num_threads(num_t);
    }

    printf("\n========================================================\n");
    printf("N: \t %d | Mode: %d | Threads: %d", N, mode, omp_get_max_threads());
    printf("\n========================================================\n");

    double *A = allocMat(N, 1);
    if (!A)
        exit(1);
    double *B = allocMat(N, 2);
    if (!B)
    {
        free(A);
        exit(1);
    }

    double *C = NULL;
    double start = 0, m_time = 0, s_time = 0, s2_time = 0, mx_time = 0, ck_time = 0;
    double sum_val, sum_val2, max_val;
    long long check_val;

    if (mode == 0)
    {
        start = omp_get_wtime();
        C = square_mat(A, B, N);
        m_time = omp_get_wtime() - start;

        start = omp_get_wtime();
        sum_val = sum_mat(C, N);
        s_time = omp_get_wtime() - start;

        start = omp_get_wtime();
        max_val = max_mat(C, N);
        mx_time = omp_get_wtime() - start;

        start = omp_get_wtime();
        check_val = checksum(C, N);
        ck_time = omp_get_wtime() - start;
    }
    else if (mode == 1)
    {
        start = omp_get_wtime();
        C = square_m1(A, B, N);
        m_time = omp_get_wtime() - start;

        start = omp_get_wtime();
        sum_val = sum_mat1(C, N);
        s_time = omp_get_wtime() - start;

        start = omp_get_wtime();
        sum_val2 = sum_mat1b(C, N);
        s2_time = omp_get_wtime() - start;

        start = omp_get_wtime();
        max_val = max_mat1(C, N);
        mx_time = omp_get_wtime() - start;

        start = omp_get_wtime();
        check_val = checksum(C, N);
        ck_time = omp_get_wtime() - start;
    }
    else if (mode == 2)
    {
        start = omp_get_wtime();
        C = square_m2(A, B, N);
        m_time = omp_get_wtime() - start;
    }
    else if (mode == 3)
    {
        C = square_m1(A, B, N);

        start = omp_get_wtime();
        check_val = checksum3A(C, N);
        printf("Atomic Checksum Time:    %f s\n", omp_get_wtime() - start);

        start = omp_get_wtime();
        check_val = checksum3B(C, N);
        printf("Critical Checksum Time:  %f s\n", omp_get_wtime() - start);
        m_time = 0;
    }
    else if (mode == 4)
    {
        start = omp_get_wtime();
        C = square_m4(A, B, N);
        m_time = omp_get_wtime() - start;
    }
    else if (mode == 5)
    {

        start = omp_get_wtime();
        C = square_m5(A, B, N);
        m_time = omp_get_wtime() - start;
    }
    else if (mode == 6)
    {
        start = omp_get_wtime();
        C = square_m6(A, B, N);
        m_time = omp_get_wtime() - start;
    }

    if (mode != 3)
    {
        sum_val = sum_mat1(C, N);
        max_val = max_mat1(C, N);
        check_val = checksum(C, N);

        printf("Analytics Summary:\n");
        printf("Multiplication: %f s\n", m_time);
        printf("  - Sum:      %.5f (Time: %f s)\n", sum_val, s_time);
        if (mode == 1)
            printf("  - Sum (dynamic):      %.5f (Time: %f s)\n", sum_val2, s2_time);
        printf("  - Max:      %.5f (Time: %f s)\n", max_val, mx_time);
        printf("  - Checksum: %lld (Time: %f s)\n", check_val, ck_time);
    }

    printf("--------------------------------------------------------\n");

    free(A);
    free(B);

    if (C != NULL)
    {

        free(C);
    }

    return 0;
}