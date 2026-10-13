# Parallel Image Processing Tool

**Student Names:** Eddie Ogendo & Jared Howse

**Course:** CS4574/CS6025 Parallel Computing

**Theme:** Theme A (Image Project)

---

## 1. Project Overview

This is the milestone report for a parallel image processing tool. Currently, the application implements a serial baseline for three primary filters:

1. **Grayscale Conversion**: Luminance-based reduction.
2. **Gaussian Blur**: 5x5 convolution kernel for noise reduction.
3. **Laplacian Edge Detection**: 3x3 kernel to identify high-frequency boundaries.

The project is structured to transition to a implement version using `MPI` and `OpenMP` parallel implementation in the next phase.

## 2. Project Structure

- `main.c`: Entry point, handles `MPI` initialization, file I/O using `stb_image`, and the primary execution logic.
- `src/`: Contains the core logic for the image processing filters.
- `makefile`: Automates the build process using mpicc.
- `resource/`: Directory containing the input image (parrot.png).

## 3. Prerequisites

Ensure the following are installed on your environment:

- `gcc` or `clang`
- `OpenMP`
- `MPI distribution` (for mpicc and mpirun)
- Standard Math Library (`-lm`)

## 4. Compilation Instructions

To compile the project, navigate to the root directory and run:

```Bash
make clean
make
```

This will create a build/release/ directory containing the main executable.

## 5. Execution Instructions

# Phase 2 Milestone: Parallel Image Processing Tool

**Student Names:** Eddie Ogendo & Jared Howse
**Course:** CS4574/CS6025 Parallel Computing
**Theme:** Theme A (Image Project)

---

## 1. Project Overview

This is the milestone report for a parallel image processing tool. Currently, the application implements a serial baseline for three primary filters:

1. **Grayscale Conversion**: Luminance-based reduction.
2. **Gaussian Blur**: 5x5 convolution kernel for noise reduction.
3. **Laplacian Edge Detection**: 3x3 kernel to identify high-frequency boundaries.

The project is structured to transition to a implement version using `MPI` and `OpenMP` parallel implementation in the next phase.

## 2. Project Structure

- `main.c`: Entry point, handles `MPI` initialization, file I/O using `stb_image`, and the primary execution logic.
- `src/`: Contains the core logic for the image processing filters.
- `makefile`: Automates the build process using mpicc.
- `resource/`: Directory containing the input image (parrot.png).

## 3. Prerequisites

Ensure the following are installed on your environment:

- `gcc` or `clang`
- `OpenMP`
- `MPI distribution` (for mpicc and mpirun)
- Standard Math Library (`-lm`)

## 4. Compilation Instructions

To compile the project, navigate to the root directory and run:

```Bash
make clean
make
```

This will create a build/release/ directory containing the main executable.

## 5. Execution Instructions

### Individual results execution
To run the serial baseline (Mode 0) with a single thread (as required for this milestone), use the following command:

```Bash

# Usage: mpirun -np {processors} ./build/release/main {mode} {threads} ./resources/<image choice.png>
mpirun -np 1 ./build/release/main 0 1 ./resources/small.png
```

To run the openMP version (Mode 1) with a chosen amount of threads, use the following command:
```Bash

# Usage: mpirun -np {processors} ./build/release/main {mode} {threads} ./resources/<image choice.png>
mpirun -np 1 ./build/release/main 1 8 ./resources/large.png
```

To run the MPI version (mode 2) with a chosen amount of processes, use the following command:
```Bash

# Usage: mpirun -np {processors} ./build/release/main {mode} {threads} ./resources/<image choice.png>
mpirun -np 4 ./build/release/main 2 0 ./resources/small.png
```

Parameters example:

- `-np 1`: Runs the process on a single `MPI` rank.
- `0`: Selects "Serial Mode."
- `1`: Sets the `OpenMP` thread count to 1.

### Benchmarking Script
This program also utilizes a Bash automation script which gives all results for serial implementation, openMP 1,2,4 and 8 threads, and MPI 1, 2, 4, and 6 processes. The script averages 5 runs after discarding the first.

To run the script first make sure its executable use the following command:

```Bash
# make the script executable
chmod +x benchmark.sh

```

To run the benchmarking script, use the following command
```Bash
# run the script
./benchmark.sh

```
This runs all required tests and image sizes 

## 6. Observed Outputs

Upon successful execution of the benchmarks script, the program will output the time taken for the computation phase and generate a csv file in the `./results/` directory:

- `benchmarks.csv`

Upon successful execution of a mode

```Bash

# Usage: mpirun -np {processors} ./build/release/main {mode} {threads} ./resources/<image choice.png>
mpirun -np 1 ./build/release/main 1 8 ./resources/large.png
```

The program will output the resulting image from the pipeline in `./rescource/output_edge.png`


## 7. Traceability / Proof of Work

Development Log can be seen on this: [Github Repository](https://github.com/edd-ie/Project1-CS4575)
