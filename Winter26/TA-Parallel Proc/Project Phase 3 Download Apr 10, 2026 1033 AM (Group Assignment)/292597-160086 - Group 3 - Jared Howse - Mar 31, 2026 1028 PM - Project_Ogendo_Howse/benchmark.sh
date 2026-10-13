#!/bin/bash

# Configuration
EXE="./build/release/main"
IMAGES=("./resource/small.png" "./resource/medium.png" "./resource/large.png" "./resource/xtra.png")
THREADS=(1 2 4 8)
PROCS=(1 2 4 6)
OUT="results/benchmarks.csv"

mkdir -p results
echo "Mode,Workload,Config,AvgTime" > $OUT

run_test() {
    local mode=$1    # 0, 1, or 2
    local config=$2  # threads or procs
    local img_path=$3
    local label=$4
    
    local total=0
    echo "Testing $label - $img_path..."

    for i in {0..4}; do
        if [ $mode -eq 2 ]; then
            # MPI Command
            TIME=$(mpirun -np $config --oversubscribe $EXE $mode 1 $img_path | grep "TIME" | awk '{print $2}')
        else
            # Serial or OMP Command
            TIME=$($EXE $mode $config $img_path | grep "TIME" | awk '{print $2}')
        fi

        # Discard run 0 (Warm-up)
        if [ $i -gt 0 ]; then
            total=$(echo "$total + $TIME" | bc -l)
        fi
    done

    avg=$(echo "$total / 4" | bc -l)
    echo "$label,$(basename $img_path),$config,$avg" >> $OUT
}

# 1. Serial Baseline
for img in "${IMAGES[@]}"; do
    run_test 0 1 "$img" "Serial"
done

# 2. OpenMP Scaling
for img in "${IMAGES[@]}"; do
    for t in "${THREADS[@]}"; do
        run_test 1 $t "$img" "OpenMP"
    done
done

# 3. MPI Scaling
for img in "${IMAGES[@]}"; do
    for p in "${PROCS[@]}"; do
        run_test 2 $p "$img" "MPI"
    done
done

echo "Benchmarking complete. Results in $OUT"