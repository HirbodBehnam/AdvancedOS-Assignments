#!/bin/bash
# CPU
OUT=$(sudo perf stat -e tlb:tlb_flush,page-faults,dtlb_load_misses.miss_causes_a_walk stress --cpu 10 --timeout 30 2>&1)
echo "$OUT"
tlb=$(echo "$OUT" | grep tlb:tlb_flush | xargs | cut -d ' ' -f1 | tr -d ,)
pagefault=$(echo "$OUT" | grep page-faults | xargs | cut -d ' ' -f1 | tr -d ,)
tlbmiss=$(echo "$OUT" | grep "dtlb_load_misses.miss_causes_a_walk" | xargs | cut -d ' ' -f1 | tr -d ,)
echo "tlb_shootdown,tlb_miss,pagefault" > 3-cpu.csv
echo "$tlb,$tlbmiss,$pagefault" >> 3-cpu.csv
sleep 5
# VM
OUT=$(sudo perf stat -e tlb:tlb_flush,page-faults,dtlb_load_misses.miss_causes_a_walk stress --vm 4 --vm-bytes 512MB --timeout 30 2>&1)
echo "$OUT"
tlb=$(echo "$OUT" | grep tlb:tlb_flush | xargs | cut -d ' ' -f1 | tr -d ,)
pagefault=$(echo "$OUT" | grep page-faults | xargs | cut -d ' ' -f1 | tr -d ,)
tlbmiss=$(echo "$OUT" | grep "dtlb_load_misses.miss_causes_a_walk" | xargs | cut -d ' ' -f1 | tr -d ,)
echo "tlb_shootdown,tlb_miss,pagefault" > 3-vm.csv
echo "$tlb,$tlbmiss,$pagefault" >> 3-vm.csv
sleep 5
# IO
OUT=$(sudo perf stat -e tlb:tlb_flush,page-faults,dtlb_load_misses.miss_causes_a_walk stress --io 5 --timeout 30 2>&1)
echo "$OUT"
tlb=$(echo "$OUT" | grep tlb:tlb_flush | xargs | cut -d ' ' -f1 | tr -d ,)
pagefault=$(echo "$OUT" | grep page-faults | xargs | cut -d ' ' -f1 | tr -d ,)
tlbmiss=$(echo "$OUT" | grep "dtlb_load_misses.miss_causes_a_walk" | xargs | cut -d ' ' -f1 | tr -d ,)
echo "tlb_shootdown,tlb_miss,pagefault" > 3-io.csv
echo "$tlb,$tlbmiss,$pagefault" >> 3-io.csv