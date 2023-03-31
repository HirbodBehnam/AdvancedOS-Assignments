#!/usr/bash
echo "threads,itlb,dtlb,stlb,pagefault,cachemiss,time" > stat.csv # empty the stat file
for i in $(seq 1 7); do
    OUT=$(sudo perf stat -e itlb.itlb_flush,tlb_flush.dtlb_thread,tlb_flush.stlb_any,page-faults,cache-misses ./3.out "$i" 2>&1)
    echo "$OUT"
    # In nultshell: Find the line, trim whitespaces, select first column and remove ',' from number
    itlb=$(echo "$OUT" | grep itlb.itlb_flush | xargs | cut -d ' ' -f1 | tr -d ,)
    dtlb=$(echo "$OUT" | grep tlb_flush.dtlb_thread | xargs | cut -d ' ' -f1 | tr -d ,)
    stlb=$(echo "$OUT" | grep tlb_flush.stlb_any | xargs | cut -d ' ' -f1 | tr -d ,)
    pagefault=$(echo "$OUT" | grep page-faults | xargs | cut -d ' ' -f1 | tr -d ,)
    cachemiss=$(echo "$OUT" | grep cache-misses | xargs | cut -d ' ' -f1 | tr -d ,)
    time_took=$(echo "$OUT" | grep "seconds time elapsed" | xargs | cut -d ' ' -f1)
    echo "$i,$itlb,$dtlb,$stlb,$pagefault,$cachemiss,$time_took" >> stat.csv
done