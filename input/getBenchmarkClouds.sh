#!/bin/bash

python3 pcd_gen.py 10000 "./clouds/test000.pcd"
prev="000"
for i in $(seq -f "%03g" 1 999); \
	do \
	python3 pcd_shift.py 500 "./clouds/test${prev}.pcd" "./clouds/test${i}.pcd" \
	prev=i
	done
