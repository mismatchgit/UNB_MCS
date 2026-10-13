#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

/* 
 * Integration function definitions
 */

// func_id = 0 : f(x) = sin(x) + 0.5*cos(3x)
double f0(double x) {
  return sin(x) + 0.5 * cos(3.0 * x);
}

// func_id = 1 : f(x) = 1 / (1 + 100*(x-0.3)^2)  — narrow peak near x=0.3
double f1(double x) {
  return 1.0 / (1.0 + 100.0 * (x - 0.3) * (x - 0.3));
}

// func_id = 2 : f(x) = sin(200x) * e^(-x)  — rapidly oscillating
double f2(double x) {
  return sin(200.0 * x) * exp(-x);
}

// Creating a function pointer type for the integrand functions
typedef double (*integrand_fn)(double);

// Function to select the appropriate function based on func_id
integrand_fn get_function(int func_id) {
  switch (func_id) {
    case 0: return f0;
    case 1: return f1;
    case 2: return f2;
    default:
      fprintf(stderr, "Error: unknown func_id %d\n", func_id);
      MPI_Abort(MPI_COMM_WORLD, 1);
      return NULL; /* unreachable, silences compiler warning */
  }
}

 
//Simpson's rule implementation for numerical integration
double simpson(integrand_fn f, double a, double b) {
  double c = (a + b) / 2.0; // Midpoint
  return (b - a) / 6.0 * (f(a) + 4.0 * f(c) + f(b)); // Simpson's rule formula
}

/**
 * Adaptive simpson's method for numerical integration
 * f: the function to integrate
 * a: the start of the interval
 * b: the end of the interval
 * whole: the Simpson's rule estimate already computed for the whole interval [a, b] 
 * tol: the desired tolerance for the approximation
 * depth: the current depth of recursion
 */
# define MAX_RECURSION_DEPTH 50 // Maximum depth of recursion to prevent infinite loops

double adaptive_simpson(integrand_fn f, double a, double b, double tol, double whole, int depth) {
  double c = (a + b) / 2.0; // Midpoint
  double left = simpson(f, a, c); // Simpson's rule on the left half
  double right = simpson(f, c, b); // Simpson's rule on the right half
  double delta = left + right - whole; // Difference between the sum of left and right and the whole interval estimate

  // Check if the difference is within the desired tolerance or if we've reached maximum recursion depth
  if (depth >= MAX_RECURSION_DEPTH || fabs(delta) <= 15.0 * tol) {
    // Return the sum of left and right as the approximation
    // delta / 15.0 is added to improve the accuracy of the approximation based on the error estimate
    return left + right + delta / 15.0; 
  }
  // Recur on the left and right halves with halved tolerance
  return adaptive_simpson(f, a, c, tol / 2.0, left, depth + 1) + 
         adaptive_simpson(f, c, b, tol / 2.0, right, depth + 1);
}

// Main integration function that uses adaptive Simpson's method
double integrate(integrand_fn f, double a, double b, double tol) {
  double whole = simpson(f, a, b); 
  return adaptive_simpson(f, a, b, tol, whole, 0);
}

// Function prototypes
void mode0_serial(integrand_fn f, double tol, int run);
void mode1_static(integrand_fn f, double tol, int rank, int size, int run);
void mode2_dynamic(integrand_fn f, double tol, int rank, int size, int run);

// Mode 0: Serial execution
void mode0_serial(integrand_fn f, double tol, int run) {
  // Only rank 0 will run this mode
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank != 0) return; // Other ranks do nothing

  double t_start = MPI_Wtime(); // Start timer
  double result = integrate(f, 0.0, 1.0, tol);
  double t_end = MPI_Wtime(); // End timer

  if (run == 0){
    printf("[Mode 0 - Serial]\n");
    printf("  Result  : %.15f\n", result);
    printf("  Time    : %.6f seconds\n", t_end - t_start);  
  }
  

}

// Mode 1: Static partitioning
#define K_PER_RANK 100 // Number of subintervals each rank will process in static mode
void mode1_static(integrand_fn f, double tol, int rank, int size, int run) {
    
    // Each rank will compute the integral over its assigned subintervals
    int K = K_PER_RANK * size; // Total number of subintervals
    double width = 1.0 / K; // Width of each subinterval

    MPI_Barrier(MPI_COMM_WORLD); // Synchronize before starting the timer
    double t_start = MPI_Wtime(); // Start timer

    double local_sum = 0.0; // Local sum for this rank
    int local_count = 0; // Count of subintervals processed by this rank

    // Each rank processes its assigned subintervals
    for (int i = rank; i < K; i+=size) {
        double a = i * width; // Start of the subinterval
        double b = (i + 1) * width; // End of the subinterval

        // tolerence is divided by K to ensure that the total error across all subintervals is within the specified tolerance
        local_sum += integrate(f, a, b, tol / K); // Compute integral for this subinterval and add to local sum
        local_count++; // Increment count of processed subintervals
    }

    double global_sum = 0.0; // Variable to hold the global sum after reduction

    // Reduce local sums to global sum on rank 0
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD); 

    int global_count = 0; // Variable to hold the total count of processed subintervals

    // Reduce local counts to global count on rank 0
    MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    double t_end = MPI_Wtime(); // End timer

    double elapsed = t_end - t_start; // Calculate elapsed time

    // Rank 0 will print the results
    if (rank == 0 && run == 0) {
        printf("[Mode 1 - Static]  P=%d  K=%d\n", size, K);
        printf("  Result          : %.15f\n", global_sum);
        printf("  Coarse intervals: %d (each rank owns %d)\n",
               global_count, K_PER_RANK);
        printf("  Time            : %.6f seconds\n", elapsed);
    }


}

// Mode 2: Dynamic load balancing
#define MAX_QUEUE 100000 // Maximum number of tasks in the queue
#define MAX_LOCAL 50000 // Max worker capacity before sending results back to master 

#define TAG_REQUEST  1 // Tag for worker requesting a task
#define TAG_TASK     2 // Tag for master sending a task to a worker
#define TAG_NEW_TASK 3 // Tag for worker sending a new task back to master (when it splits and generates subtasks)
#define TAG_RESULT   4 // Tag for worker sending a result back to master
#define TAG_STOP     5 // Tag for master telling workers to stop
void mode2_dynamic(integrand_fn f, double tol, int rank, int size, int run) {
   if (size < 2){
    if (rank == 0) fprintf(stderr, "Error: Dynamic mode requires at least 2 processes\n");
    return;
   }

   MPI_Barrier(MPI_COMM_WORLD); // Synchronize before starting the timer
   double t_start = MPI_Wtime(); // Start timer

   // Buffer for sending/receiving tasks and results
   double buf[3];
   MPI_Status st;

   // Master process (rank 0) will distribute tasks
   if (rank == 0) {
    int num_workers = size - 1; // Number of worker processes

    double *tq = malloc(MAX_QUEUE * 3 * sizeof(double)); // Task queue: each task is [a, b, tol]
    if (!tq) {
      fprintf(stderr, "Error: unable to allocate task queue\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int qtop = 0; // Top of the task queue

    tq[qtop*3+0] = 0.0; // Start of the interval
    tq[qtop*3+1] = 1.0; // End of the interval
    tq[qtop*3+2] = tol; // Tolerance for this task
    qtop++; // Increment task queue top

    int *waiting = malloc(num_workers * sizeof(int)); // Track which workers are waiting for tasks
    int nwait = 0; // Number of waiting workers
    int active = 0; // Workers currently holding a task

    double total = 0.0; // Total integral result
    long n_accept = 0; // Count of accepted tasks

    while (1) {
      while (nwait > 0 && qtop > 0) {
        // Assign tasks to waiting workers
        int w = waiting[--nwait]; // Get a waiting worker
        qtop--; // Get a task from the queue
        MPI_Send(tq + (qtop*3), 3, MPI_DOUBLE, w, TAG_TASK, MPI_COMM_WORLD); // Send task to worker
        active++; // Increment active worker count
      }

      // Termination check
      // Stop only when there are no more tasks in the queue, no active workers, and all workers are waiting (i.e., done)
      if(qtop == 0 && active == 0 && nwait == num_workers) {
         for (int i = 0; i < nwait; i++) {
          MPI_Send(buf, 3, MPI_DOUBLE, waiting[i], TAG_STOP, MPI_COMM_WORLD); // Send stop signal to all waiting workers
         }
          break; // Exit the loop
      }

      // Wait for any worker to send a message (either a result or a new task)
      MPI_Recv(buf, 3, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &st); // Receive messages from workers

      switch (st.MPI_TAG) {
        
        case TAG_REQUEST:
        // Worker is requesting a task, add it to the waiting list and assign a task if available in the next iteration
          waiting[nwait++] = st.MPI_SOURCE;
          break;

        case TAG_RESULT:
        // Worker has completed a task and sent back a result
          total += buf[0]; // Add the result to the total
          active--; // Decrement active worker count
          n_accept += (long)buf[1]; // Increment accepted task count
          break;

        case TAG_NEW_TASK:
        // Worker has split a task into subtasks and is sending them back to the master
          if (qtop < MAX_QUEUE) {
            tq[qtop * 3 + 0] = buf[0]; // Start of new task
            tq[qtop * 3 + 1] = buf[1]; // End of new task
            tq[qtop * 3 + 2] = buf[2]; // Tolerance for new task
            qtop++; // Increment task queue top
          }
          break;
      }
    }

    double elapsed = MPI_Wtime() - t_start; // Calculate elapsed time

    if (run == 0) {
      printf("[Mode 2 - Dynamic]  P=%d\n", size);
      printf("  Result            : %.15f\n", total);
      printf("  Accepted intervals: %ld\n",   n_accept);
      printf("  Time              : %.6f seconds\n", elapsed);
    }

    free(tq); // Free allocated memory for task queue
    free(waiting); // Free allocated memory for waiting list
  }
  else {
    // Worker Processes block

    typedef struct 
    {
      double l, r, tol, whole; int depth;
    } LTask; // Structure to represent a task for local adaptive integration

    LTask *ls = malloc(MAX_LOCAL * sizeof(LTask)); // Local stack for tasks
    if (!ls) {
      fprintf(stderr, "Error: unable to allocate local task stack\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    while (1) {
      // Request a task from the master
      MPI_Send(buf, 3, MPI_DOUBLE, 0, TAG_REQUEST, MPI_COMM_WORLD);
      
      // Receive a task from the master or a stop signal
      MPI_Recv(buf, 3, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &st);

      // Check if it's a stop signal
      if (st.MPI_TAG == TAG_STOP) break; // Exit the loop and terminate
      
      // Process task using local stack (not using recursion to avoid deep call stacks and stack overflow issues)
      double local_result = 0.0;
      long local_count = 0;
      int ltop = 0; // Local stack top index

      // Unpack the received task parameters
      double a0 = buf[0], b0 = buf[1], tol0 = buf[2]; 
      ls[ltop++] = (LTask){a0, b0, tol0, simpson(f, a0, b0), 0}; // Push initial task onto local stack

      while (ltop > 0) {
        LTask curr = ls[--ltop]; // Pop a task from local stack

        double m = (curr.l + curr.r) / 2.0; // Midpoint of the current interval
        double left = simpson(f, curr.l, m); // Simpson's rule on the left half
        double right = simpson(f, m, curr.r); // Simpson's rule on the right half
        double delta = left + right - curr.whole; // Difference between the sum of left and right and the whole interval estimate

        // Check if the difference is within the desired tolerance (acceptable result)
        // or if we've reached maximum recursion depth
        if (curr.depth >= MAX_RECURSION_DEPTH || fabs(delta) <= 15.0 * curr.tol ) {

          // Add result in local sum and increment count of accepted intervals
          local_result += left + right + delta / 15.0; 
          local_count++;
        }
        else {
          // Rejected case

          // Send right half back to master as new task and  keep left half on local stack
          buf[0] = m; // Start of the new task interval (right half)
          buf[1] = curr.r; // End of the new task interval
          buf[2] = curr.tol / 2.0; // Halve the tolerance for the new task

          MPI_Send(buf, 3, MPI_DOUBLE, 0, TAG_NEW_TASK, MPI_COMM_WORLD);

          // Push left half back onto local stack for further processing
          if(ltop < MAX_LOCAL) {
            ls[ltop++] = (LTask){curr.l, m, curr.tol / 2.0, left, curr.depth + 1};
          }
      }
    }

    // Report accumulated local result back to master
    buf[0] = local_result;
    buf[1] = (double)local_count;
    buf[2] = 0.0; // Unused for result message

    MPI_Send(buf, 3, MPI_DOUBLE, 0, TAG_RESULT, MPI_COMM_WORLD);
    
  }

    free(ls); // Free allocated memory for local task stack
  }
}

// Main function

int main(int argc, char *argv[]) {

  // Initialize MPI
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Check command line arguments
  // Only rank 0 will print the usage message if the arguments are incorrect
  if (argc != 4) {
    if (rank == 0) {
      fprintf(stderr,
                "Usage: mpirun -np P ./integration func_id mode tol\n"
                "  func_id : 0, 1, or 2\n"
                "  mode    : 0=serial, 1=static, 2=dynamic\n"
                "  tol     : e.g. 1e-6 or 1e-8\n");
    }
    MPI_Finalize();
    return 1;
  }

  int func_id = atoi(argv[1]);
  int mode = atoi(argv[2]);
  double tol = atof(argv[3]);

  // Get the function pointer based on func_id
  integrand_fn f = get_function(func_id);

  // Print header information only on rank 0
  if (rank == 0) {
    printf("==============================================\n");
    printf(" MPI Adaptive Integration\n");
    printf(" func_id=%d  mode=%d  tol=%.2e  P=%d\n", func_id, mode, tol, size);
    printf("==============================================\n");
  }

  // Call function 10 times 

  double times[10]; // Array to store execution times for each run

  for (int run = 0; run < 10; run++) {
    MPI_Barrier(MPI_COMM_WORLD); // Synchronize before starting the timer
    double t0 = MPI_Wtime(); // Start timer
  

    // Call the appropriate mode function
    switch (mode) {
      case 0: mode0_serial(f, tol, run); break;
      case 1: mode1_static(f, tol, rank, size, run); break;
      case 2: mode2_dynamic(f, tol, rank, size, run); break;
      default:
        if (rank == 0) {
          fprintf(stderr, "Error: unknown mode %d\n", mode);
        }
        MPI_Finalize();
        return 1;
    }

    MPI_Barrier(MPI_COMM_WORLD); // Synchronize after the run
    times[run] = MPI_Wtime() - t0;

  }

  // Compute average of the 9 runs
  if (rank == 0) {
    double sum = 0.0;
    for (int i = 1; i < 10; i++) { // Skip the first run
      sum += times[i];
    }
    double avg = sum / 9.0;
    printf("--------------------------------------------------\n");
    printf("  Timing summary (discarded run 0)\n");
    for (int r = 0; r < 10; r++)
      printf("    run %2d: %.6f s%s\n",
             r, times[r], r == 0 ? "  [discarded]" : "");
    printf("  Average (runs 1-%d): %.6f seconds\n", 9, avg);
    printf("==================================================\n");
  }

  // Finalize MPI
  MPI_Finalize();
  return 0;
  
}




