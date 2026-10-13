# Assignment 3
## Hybrid MPI + OpenMP Adaptive Numerical Integration

**Name:** Ruturaj Sushil Ghodke 
**ID:**  3775789

---

## Files

```
A3_Ghodke_Ruturaj.zip
├── integration.c   — complete program (Mode 0 serial, Mode 1 MPI dynamic, Mode 2 hybrid)
├── report.pdf         — written report
├── README.md          — this file
└── Makefile           — build file
```

---

## Compile

```bash
make
```

Or manually:
```bash
mpicc -O3 -fopenmp integration.c -o integration -lm
```

---

## Usage

```
mpirun -np P ./integration func_id mode tol
```

| Argument | Values | Description |
|----------|--------|-------------|
| `P` | ≥ 1 | number of MPI processes |
| `func_id` | 0, 1, 2 | function to integrate |
| `mode` | 0, 1, 2 | execution mode |
| `tol` | e.g. `1e-6`, `1e-8` | error tolerance |

### Functions

| ID | f(x) |
|----|------|
| 0 | sin(x) + 0.5·cos(3x) |
| 1 | 1 / (1 + 100·(x−0.3)²) |
| 2 | sin(200x)·e^(−x) |

### Modes

| Mode | Description |
|------|-------------|
| 0 | Serial baseline — single process, no parallelism |
| 1 | MPI dynamic master/worker — global task queue with load balancing |
| 2 | Hybrid MPI+OpenMP — static distribution + OpenMP tasks per worker |

> Mode 1 and Mode 2 require at least 2 MPI processes.

---

## Example Run Commands

```bash
# Mode 0 — serial baseline
mpirun -np 1 ./integration 1 0 1e-6

# Mode 1 — MPI dynamic (function 1, 4 processes, tol=1e-8)
mpirun -np 4 ./integration 1 1 1e-8

# Mode 2 — hybrid (function 1, P=4, T=2 threads, tol=1e-6)
OMP_NUM_THREADS=2 mpirun -np 4 ./integration 1 2 1e-6

# Mode 2 — hybrid (function 0, P=2, T=4 threads, tol=1e-8)
OMP_NUM_THREADS=4 mpirun -np 2 ./integration 0 2 1e-8

# Mode 2 — hybrid (function 0, P=4, T=4 threads, tol=1e-8)
OMP_NUM_THREADS=4 mpirun -np 4 ./integration 1 2 1e-8
```

---

## Machine Specifications

| Item | Details |
|------|---------|
| Machine | Intel Core Ultra 7 |
| OS | Ubuntu (WSL2 on Windows) |
| Physical cores | 16 |
| MPI implementation | OpenMPI |
| OpenMP runtime | GCC built-in |
| Compiler | mpicc (GCC 13.2.0), |

---

## Notes

- Each run executes the mode 10 times internally; run 0 is discarded and the average of runs 1–9 is reported.
- Mode 2 hybrid uses `OMP_NUM_THREADS` to set the number of OpenMP threads per MPI process. Total parallelism = P × T.
- For f2 (sin(200x)·e^(−x)) at tol=1e-6, the serial adaptive algorithm hits MAX_DEPTH=50 and returns an inaccurate result. Correctness verification uses f0 and f1.

---
