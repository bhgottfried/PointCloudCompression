#!/bin/bash

python3 pcd_gen.py 5 "./small/test0000.pcd"
prev="0000"
for i in $(seq -f "%04g" 1 5); do
	python3 pcd_shift.py 1 "./small/test${prev}.pcd" "./small/test${i}.pcd"
	prev=$i
done
