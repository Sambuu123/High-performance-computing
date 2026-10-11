#!/usr/bin/env bash
# Бүх хэмжилтийг OMP_NUM_THREADS = 1, 2, 4, 8 дээр ажиллуулж
# results/results.csv (-O3) ба results/race_O0.csv (B2, -O0) файлд хадгална.
set -euo pipefail
cd "$(dirname "$0")"

make -s lab3 lab3_O0
mkdir -p results

header="variant,threads,run,time,value"

echo "$header" > results/results.csv
for t in 1 2 4 8; do
    echo "=== OMP_NUM_THREADS=$t (-O3) ==="
    OMP_NUM_THREADS=$t ./lab3 | tee "results/log_t$t.txt" \
        | grep '^RESULT,' | cut -d, -f2- >> results/results.csv
done

echo "$header" > results/race_O0.csv
for t in 2 4 8; do
    echo "=== OMP_NUM_THREADS=$t (-O0, B2) ==="
    OMP_NUM_THREADS=$t ./lab3_O0 B2 | tee "results/log_O0_t$t.txt" \
        | grep '^RESULT,' | cut -d, -f2- | grep -v '^INIT_PAR' >> results/race_O0.csv
done

{
    echo "CPU:      $(lscpu | sed -n 's/^Model name:[[:space:]]*//p')"
    echo "Цөм:      $(lscpu | sed -n 's/^Core(s) per socket:[[:space:]]*//p') физик, $(nproc) логик"
    echo "RAM:      $(free -h | awk '/^Mem:/ {print $2}')"
    echo "Компайлер: $(gcc --version | head -1)"
    echo "OS:       $(uname -sr)"
    echo
    numactl --hardware 2>/dev/null || lscpu | grep -i numa
} > results/system.txt

echo "Дууслаа: results/results.csv, results/race_O0.csv, results/system.txt"
