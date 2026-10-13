# Assignment 2
## MPI Adaptive Numerical Integration with Dynamic Load Balancing

**Name:** Ruturaj Ghodke  
**ID:**  3775789

---

## Files

```
A2_Ghodke_Ruturaj.zip
├── integration.c   — complete MPI program (all 3 modes)
├── report.pdf      — written report with tables, plots, and discussion
└── README.md       — this file
```

---

## Compile Command

```bash
mpicc -O3 integration.c -o integration -lm
```

---

## Program Usage

```
mpirun -np P ./integration func_id mode tol
```

| Argument | Values | Description |
|----------|--------|-------------|
| `P`       | any integer ≥ 1 | number of MPI processes |
| `func_id` | 0, 1, 2 | function to integrate (see below) |
| `mode`    | 0, 1, 2 | execution mode (see below) |
| `tol`     | e.g. `1e-6`, `1e-8` | error tolerance |

### Functions

| ID | f(x) |
|----|------|
| 0 | sin(x) + 0.5·cos(3x) |
| 1 | 1 / (1 + 100·(x−0.3)²) |
| 2 | sin(200x)·e^(−x) |

### Modes

| Mode | Description |
|------|-------------|
| 0 | Serial baseline — single process, no MPI parallelism |
| 1 | Static decomposition — intervals split evenly across ranks |
| 2 | Dynamic master/worker — task queue with adaptive load balancing |

> **Note:** Mode 2 requires at least 2 processes (1 master + 1 worker).

---

## Example Run Commands

```bash
# Mode 0 — serial baseline (function 2, tolerance 1e-6)
mpirun -np 1 ./integration 2 0 1e-6

# Mode 1 — static decomposition (function 2, 4 processes, tolerance 1e-8)
mpirun -np 4 ./integration 2 1 1e-8

# Mode 2 — dynamic master/worker (function 2, 8 processes, tolerance 1e-8)
mpirun -np 8 ./integration 2 2 1e-8



---

## Timing

Each run executes the selected mode **10 times** internally.  
Run 0 is discarded (MPI channel warm-up).  
The program reports the average of runs 1–9 along with min/max.

---

## Machine Specifications
 
| Item | Details |
|------|---------|
| Machine | Intel Core Ultra 7 |
| OS | Ubuntu 24 (WSL2 on Windows) |
| Cores | 8 physical cores |
| MPI implementation | OpenMPI |
| Compiler | mpicc (GCC backend) |
| Optimisation flag | -O3 |