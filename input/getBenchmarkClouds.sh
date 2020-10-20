#!/bin/bash

python3 pcd_gen.py 1000 "./clouds/test0000.pcd"
prev="0000"
for i in $(seq -f "%04g" 1 8096); \
	do \
	python3 pcd_shift.py 50 "./clouds/test${prev}.pcd" "./clouds/test${i}.pcd" \
	prev=i
	done
