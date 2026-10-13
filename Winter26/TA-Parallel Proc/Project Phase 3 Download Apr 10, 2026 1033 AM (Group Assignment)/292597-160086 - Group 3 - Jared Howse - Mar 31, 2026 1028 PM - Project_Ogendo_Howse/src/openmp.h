#ifndef OPENMP_H
#define OPENMP_H

#include <omp.h>
#include <math.h>
#include <stdlib.h>
void omp_grayscale(unsigned char *const img, unsigned char *const gray_img, const int height, const int width)
{
    const int total_pixels = height * width;

/*
 * Use a 1D loop to match the 1D memory layout
 * 'simd' explicitly tells the compiler to vectorize the math
 * Reduced Instruction Count: removed two multiplications (y * width) and one addition for
 *  every single pixel.
 * Cache Friendliness: A 1D loop guarantees the CPU pre-fetcher stays on a linear path,
 *  reducing cache misses.
 * Hardware Acceleration: The simd directive encourages the use of 256-bit or 512-bit registers,
 *  potentially providing a 4x–8x speedup over a standard parallel loop.
 */
#pragma omp parallel for simd default(none) shared(img, gray_img, total_pixels) schedule(static)
    for (int i = 0; i < total_pixels; i++)
    {
        int in_idx = i * 4;

        // temporary variables to help the compiler identify independent loads
        unsigned char r = img[in_idx];
        unsigned char g = img[in_idx + 1];
        unsigned char b = img[in_idx + 2];

        // Linear storage for 1-channel output
        gray_img[i] = (unsigned char)(0.299f * r + 0.587f * g + 0.114f * b);
    }
}

void omp_gaussian_blur(unsigned char *const in, unsigned char *out, int w, int h)
{
    static const int kernel[5][5] = {
        {1, 4, 7, 4, 1},
        {4, 16, 26, 16, 4},
        {7, 26, 41, 26, 7},
        {4, 16, 26, 16, 4},
        {1, 4, 7, 4, 1}};
    static const int kernel_sum = 273;
    /*
     * default(none) - ensure no unused variable are copied during execution
     * shared(in, out, w, h) - specifing data to be duplicated to each thread'
     * schedule(static)
     * collapse(n) - loops collapsed into one large iteration space and divided according to the
     *  schedule at image is accessed a 1D array
     */
#pragma omp parallel for default(none) shared(in, out, w, h, kernel) collapse(2) schedule(static)
    for (int y = 2; y < h - 2; y++)
    {
        for (int x = 2; x < w - 2; x++)
        {
            int sum = 0;

            // Iterate through the 5x5 neighborhood
            for (int ky = -2; ky <= 2; ky++)
            {
                // Pre calculating these values before the inner for loop to save time
                int row = (y + ky) * w;
                int krow = ky + 2;
                for (int kx = -2; kx <= 2; kx++)
                {
                    int pixel = in[row + (x + kx)];
                    sum += pixel * kernel[krow][kx + 2];
                }
            }
            out[y * w + x] = (unsigned char)(sum / kernel_sum);
        }
    }
}

/*
 * Eliminates Loop Overhead: You avoid 9 ky/kx iterations per pixel.
 * Pointer Arithmetic: By using prev_row, curr_row, and next_row,
 *  replaces complex index math like (y + ky) * width + (x + kx) with simple offsets.
 */
void omp_edge_detect(unsigned char *const in, unsigned char *out, const int width, const int height)
{
// Use a pointer-based approach to minimize multiplications in the inner loop
#pragma omp parallel for default(none) shared(in, out, width, height) schedule(static)
    for (int y = 1; y < height - 1; y++)
    {
        // Pre-calculate row pointers for the 3 rows we need
        const unsigned char *prev_row = &in[(y - 1) * width];
        const unsigned char *curr_row = &in[y * width];
        const unsigned char *next_row = &in[(y + 1) * width];
        unsigned char *out_row = &out[y * width];

#pragma omp simd
        for (int x = 1; x < width - 1; x++)
        {
            // Manual unrolling of the Laplacian kernel: {0, 1, 0}, {1, -4, 1}, {0, 1, 0}
            int sum = prev_row[x]          // Top
                      + curr_row[x - 1]    // Left
                      + curr_row[x + 1]    // Right
                      + next_row[x]        // Bottom
                      - (curr_row[x] * 4); // Center

            int edge = abs(sum);
            out_row[x] = (unsigned char)(edge > 255 ? 255 : edge);
        }
    }
}

#endif // OPENMP_H