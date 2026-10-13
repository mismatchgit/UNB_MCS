# Assignment 1 – OpenMP Matrix Multiplication + SIMD + Performance Analysis

This program implements several variants of matrix multiplication using OpenMP, as described in the assignment. Contains 6 Modes, The assignement details can be found in ./assignment folder.

## Problem Sizes Used

- **Small:** `N = 512`
- **Medium:** `N = 1024`
- **Large:** `N = 2048`

Each run multiplies two `N × N` matrices and computes:

- The resulting matrix `C = A × B`
- The sum of all elements in `C`
- The maximum element in `C`
- CheckSum += (long long)(C[i*N + j] \* 1000.0) % 100000;

## Compilation Command

gcc-15 -O3 -fopenmp a1.c -lm -o a1

## Running Program command

./a1 N mode

    Example:
    ./a1 512 6 (small size, mode 6)
    ./a1 1024 0 (medium size, mode 0)

## Machine Specifucation

- Operating System:macOS
- CPU: 2.3 GHz 8-Core Intel Core i9
- Logical Cores (Threads): 16 hardware threads (8 cores × 2 threads)
- RAM: 16 GB 2667 MHz DDR4

## Authorship Statement

I Ishimwe Pacis Hanyurwimfura confirm that :

- All code in this repository was written and understood by me
- I implemented each mode (0–6) incrementally, and used git commits to track my progress
- Any external resources I consulted were for general OpenMP and C language reference, and no code was directly copied from other students or online solutions.

## LINK to Github

https://github.com/Pax-02/assignment1-intro-to-parallel-process.git
