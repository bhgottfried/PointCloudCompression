import sys
from random import random
from array import array

if len(sys.argv) < 2:
	print("usage: python3 pcd_gen.py <number of points>")
	exit()

numPoints = int(sys.argv[1])

with open("rand{0}.pcd".format(numPoints), "wb") as outFile:
	outFile.write(("VERSION .7\n\
					FIELDS x y z\n\
					SIZE 4 4 4\n\
					TYPE f f f\n\
					COUNT 1 1 1\n\
					WIDTH {0}\n".format(numPoints)
				+  "HEIGHT 1\nVIEWPOINT 0 0 0 1 0 0 0\n\
					POINTS {0}\nDATA binary\n".format(numPoints)).encode("ascii"))
	array("f", [random() * 2 - 1 for _ in range(numPoints * 3)]).tofile(outFile)
