#!/bin/bash
echo "threads,tlb-shootdown,tlb-miss,pagefault,time" > stat.csv # empty the stat file
for i in $(seq 1 7); do
    OUT=$(sudo perf stat -e tlb:tlb_flush,page-faults,dtlb_load_misses.miss_causes_a_walk ./1.out "$i" 2>&1)
    echo "$OUT"
    # In nultshell: Find the line, trim whitespaces, select first column and remove ',' from number
    tlb=$(echo "$OUT" | grep tlb:tlb_flush | xargs | cut -d ' ' -f1 | tr -d ,)
    pagefault=$(echo "$OUT" | grep page-faults | xargs | cut -d ' ' -f1 | tr -d ,)
    tlbmiss=$(echo "$OUT" | grep "dtlb_load_misses.miss_causes_a_walk" | xargs | cut -d ' ' -f1 | tr -d ,)
    time_took=$(echo "$OUT" | grep "seconds time elapsed" | xargs | cut -d ' ' -f1)
    echo "$i,$tlb,$tlbmiss,$pagefault,$time_took" >> stat.csv
done