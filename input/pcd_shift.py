import sys
import random
import struct
from array import array
from operator import itemgetter

# Parse args
if len(sys.argv) < 3:
	print("usage: python3 pcd_shift.py <number of points to shift> <.pcd file to shift> <output location>")
	exit()

# Read data
with open(sys.argv[2], "rb") as inFile:
    header = [inFile.readline() for _ in range(10)]
    data = inFile.read()

# Create an array of floats from data and sort it
points = [[*struct.unpack("3f", data[i:i+12])] for i in range(0, len(data), 12)]
points.sort(key = itemgetter(0,1,2))

# Shift all points of a random, adjacent block of points
shiftSlice = random.randint(0, int(len(points) - 1 - int(sys.argv[1])))
for i in range(shiftSlice, shiftSlice + int(sys.argv[1])):
    for j in range(3):
        # Shift each coordinate by a random amount within plus/minus 5%
        points[i][j] += (random.random() / 10 - 0.05) * points[i][j]

# Write header and shifted data to new .pcd file
with open(sys.argv[3], "wb") as outFile:
    outFile.writelines(header)
    for point in points:
        outFile.write(struct.pack("3f", *point))
