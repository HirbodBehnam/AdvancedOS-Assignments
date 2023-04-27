#!/bin/bash
echo "threads,tlb,pagefault,time" > stat.csv # empty the stat file
for i in $(seq 1 7); do
    OUT=$(sudo perf stat -e tlb:tlb_flush,page-faults ./1.out "$i" 2>&1)
    echo "$OUT"
    # In nultshell: Find the line, trim whitespaces, select first column and remove ',' from number
    itlb=$(echo "$OUT" | grep tlb:tlb_flush | xargs | cut -d ' ' -f1 | tr -d ,)
    pagefault=$(echo "$OUT" | grep page-faults | xargs | cut -d ' ' -f1 | tr -d ,)
    time_took=$(echo "$OUT" | grep "seconds time elapsed" | xargs | cut -d ' ' -f1)
    echo "$i,$itlb,$pagefault,$time_took" >> stat.csv
done