#!/bin/bash
echo "test,itlb,dtlb,stlb" > stat.csv # empty the stat file
for i in {1..100}; do
    OUT=$(sudo perf stat -e itlb.itlb_flush,tlb_flush.dtlb_thread,tlb_flush.stlb_any ./2.out 2>&1)
    echo "$OUT"
    # In nultshell: Find the line, trim whitespaces, select first column and remove ',' from number
    itlb=$(echo "$OUT" | grep itlb.itlb_flush | xargs | cut -d ' ' -f1 | tr -d ,)
    dtlb=$(echo "$OUT" | grep tlb_flush.dtlb_thread | xargs | cut -d ' ' -f1 | tr -d ,)
    stlb=$(echo "$OUT" | grep tlb_flush.stlb_any | xargs | cut -d ' ' -f1 | tr -d ,)
    echo "$i,$itlb,$dtlb,$stlb" >> stat.csv
done