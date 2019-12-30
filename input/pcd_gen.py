from random import random
from array import array

numPoints = 10

with open("rand{0}.pcd".format(numPoints), "wb") as outFile:
	outFile.write(("VERSION .7\nFIELDS x y z\nSIZE 4 4 4\nTYPE f f f\nCOUNT 1 1 1\nWIDTH {0}\n".format(numPoints)
				+ "HEIGHT 1\nVIEWPOINT 0 0 0 1 0 0 0\nPOINTS {0}\nDATA binary\n".format(numPoints)).encode("ascii"))
	array("f", [random() * 2 - 1 for _ in range(numPoints * 3)]).tofile(outFile)
