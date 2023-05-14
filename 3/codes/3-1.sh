#!/bin/bash
fio --filename="/media/hirbod/Linux External/file.txt" --direct=1 --rw=write --io_size=20G --bs=4MB --ioengine=libaio --iodepth=8 --numjobs=1 --group_reporting --size=20GB --name=iops_test_job --eta-newline=1 --output=seq-write-20.txt
