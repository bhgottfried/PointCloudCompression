#!/bin/bash

mkdir input/clouds
cd input
./getBenchmarkClouds.sh

cd ../
mkdir output
touch output/serial.pcs
touch output/parallel.pcs

cd serial
make opt > /dev/null

cd ../parallel
make opt > /dev/null
cd ../

echo "Setup complete. cd into either the 'parallel' or 'serial' directory, then type 'make run' to benchmark."
