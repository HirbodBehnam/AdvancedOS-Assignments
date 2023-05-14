#!/bin/bash
fio --filename="/media/hirbod/Linux External/file.txt" --random_distribution=zipf:1.2 --direct=1 --rw=randread --io_size=4G --bs=4MB --ioengine=libaio --iodepth=16 --numjobs=1 --group_reporting --name=iops_test_job --eta-newline=1 --output=random-read.txt
