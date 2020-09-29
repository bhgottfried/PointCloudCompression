import sys
from random import random
from array import array

if len(sys.argv) < 2:
	print("usage: python3 pcd_gen.py <number of points> <output location>")
	exit()

numPoints = int(sys.argv[1])

with open(sys.argv[2], "wb") as outFile:
	outFile.write(("VERSION .7\n"
				+ "FIELDS x y z\n"
				+ "SIZE 4 4 4\n"
				+ "TYPE f f f\n"
				+ "COUNT 1 1 1\n"
				+ "WIDTH {0}\n".format(numPoints)
				+ "HEIGHT 1\nVIEWPOINT 0 0 0 1 0 0 0\n"
				+ "POINTS {0}\n".format(numPoints)
				+ "DATA binary\n").encode("ascii"))
	array("f", [random() * 2 - 1 for _ in range(numPoints * 3)]).tofile(outFile)
