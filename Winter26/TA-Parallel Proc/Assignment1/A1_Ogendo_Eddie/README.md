# Assignment 1: OpenMP Matrix Multiplication

## CS4575/CS6025: Parallel Computing

### 1. Project Overview

This project implements square matrix multiplication ($C = A \times B$) using various OpenMP parallelization strategies, including work-sharing loops, task-based decomposition, and SIMD vectorization.

### 2. Authorship & Academic Integrity

**Author:** I confirm that the code in Asn1.c was developed incrementally by me. The Git history provided reflects the actual development stages of the project.

### 3. Compilation & Build

To compile the program using the provided Makefile:

```Bash
make
```

_Note: This uses gcc with the -O3 optimization flag and -fopenmp for parallel support._

### 4. Usage

The executable accepts two arguments: Matrix Dimension ($N$) and Execution Mode.

```Bash
./Asn1 <N> <mode>
```

Execution Modes

- 0 - Serial Baseline (Triple Loop)
- 1 - Threaded: Standard parallel for (Dynamic Scheduling)
- 2 - Threaded: Nested Loops with collapse(2)
- 3 - Synchronization: Atomic vs Critical Checksum Comparison
- 4 - Tasking: Explicit omp task with Row-Blocking
- 5 - SIMD: Single-threaded Vectorization
- 6 - Hybrid: parallel for simd (Threads + Vectorization)

### 5. Automation

#### Single test runs

You can use the following make targets to run specific modes with the default $N=512$:

- Run Mode 0: `make run0`
- Run Mode 1: `make run1`
- Run Mode 2: `make run2`
- Run Mode 3: `make run3`
- Run Mode 4: `make run4`
- Run Mode 5: `make run5`
- Run Mode 6: `make run6`

**Memory Leak Verification**

To ensure proper allocation and deallocation of matrices:

```Bash
make memcheck
```

**Cleanup**

To remove the compiled binary:

```Bash
make clean
```

#### Batch run & Data Collection

To satisfy the repetition protocol (11 runs, discarding the first), use the provided collection script:

```Bash
chmod +x collect_data.sh && ./collect_data.sh
```

This script generates:

- `results/raw_runs.csv`: Every individual timing run.
- `results/timings.csv`: Aggregated averages for report plotting.

### 6. Experimental Setup

```
Architecture:             x86_64
  CPU op-mode(s):         32-bit, 64-bit
  Address sizes:          39 bits physical, 48 bits virtual
  Byte Order:             Little Endian

CPU(s):                   8
  On-line CPU(s) list:    0-7
  Model name:             Intel(R) Core(TM) i5-8250U CPU @ 1.60GHz
    CPU family:           6
    Model:                142
    Thread(s) per core:   2
    Core(s) per socket:   4

Virtualization features:
  Hypervisor vendor:      Microsoft
  Virtualization type:    full

Caches (sum of all):
  L1d:                    128 KiB (4 instances)
  L1i:                    128 KiB (4 instances)
  L2:                     1 MiB (4 instances)
  L3:                     6 MiB (1 instance)

NUMA:
  NUMA node(s):           1
  NUMA node0 CPU(s):      0-7
```

### 7. Authorship Trail (Incremental Development)

```plaintext
4953afb 2026-02-26 documentation updates
710ea13 2026-02-26 Update to thread pool count
3844fdd 2026-02-26 Finalized main execution logic, build system, and documentation
d2410c4 2026-02-25 Integrated Mode 5/6 SIMD and parallelized analytic functions
c437e89 2026-02-22 Added Mode 3 (Synchronization comparison) and Mode 4 (Task-based blocking)
b702e59 2026-02-19 Implemented Mode 1 (Work-sharing) and Mode 2 (Collapse) with proper scoping
b9d468c 2026-02-15 Added Mode 0: Standard serial triple-loop matrix multiplication
```
