#!/bin/bash
for f in 2-*.csv ; do
    itlb=$(awk -F',' '{sum+=$2; ++n} END { print sum/n }' < "$f")
    dtlb=$(awk -F',' '{sum+=$3; ++n} END { print sum/n }' < "$f")
    stlb=$(awk -F',' '{sum+=$4; ++n} END { print sum/n }' < "$f")
    echo "${f%.*} & $itlb & $dtlb & $stlb \\\\" 
done