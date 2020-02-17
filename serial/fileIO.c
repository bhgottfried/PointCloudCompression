#include "octree.h"


// Write a pcd header with the number of points of the compressed octree
void write_header(FILE* const fp, unsigned int numPoints)
{
	const char version[] = "VERSION .7\n";
	const char fields[] = "FIELDS x y z\n";
	const char size[] = "SIZE 4 4 4\n";
	const char type[] = "TYPE f f f\n";
	const char count[] = "COUNT 1 1 1\n";
	char width[24] = "WIDTH ";		// Still need to convert numPoints into ASCII
	const char height[] = "HEIGHT 1\n";
	const char viewpoint[] = "VIEWPOINT 0 0 0 1 0 0 0\n";
	char points[24] = "POINTS ";	// Still need to convert numPoints into ASCII
	const char data[] = "DATA binary\n";

	unsigned int numDigits = 0;
	unsigned int numPointsCpy = numPoints;
	while (numPointsCpy)
	{
		numPointsCpy /= 10;
		numDigits++;
	}

	for (int digIdx = numDigits - 1; digIdx >= 0; digIdx--)
	{
		char currDigit = numPoints % 10 + '0';
		numPoints /= 10;
		width[digIdx + 6] = currDigit;
		points[digIdx + 7] = currDigit;
	}

	width[numDigits + 6] = '\n';
	width[numDigits + 7] = '\0';
	points[numDigits + 7] = '\n';
	points[numDigits + 8] = '\0';

	const char* adrs[] = { version, fields, size, type, count, width, height, viewpoint, points, data };
	for (int adrIdx = 0; adrIdx < 10; adrIdx++)
	{
		fwrite(adrs[adrIdx], sizeof(**adrs), strlen(adrs[adrIdx]), fp);
	}
}


// Depth first traversal of octree, writing centroids of points when at leaf nodes
void write_octree_points(FILE* const fp, const OctreeNode* const root, float lx, float ux, float ly, float uy, float lz, float uz)
{
	if (root)
	{
		float midx = lx + (ux - lx) / 2, midy = ly + (uy - ly) / 2, midz = lz + (uz - lz) / 2;	// Midpoints of bounds
		if (root->isLeaf)	// root is leaf node so write centroids to file
		{
			float centroid[] = { midx, midy, midz };
			fwrite(centroid, sizeof(*centroid), 3, fp);
		}
		else
		{
			for (int childIdx = 0; childIdx < 8; childIdx++)
			{
				if (root->children[childIdx])
				{
					float nlx = lx, nux = ux, nly = ly, nuy = uy, nlz = lz, nuz = uz;	// Get new bounds for suboctant
					if (childIdx & 4)
						nlx = midx;
					else
						nux = midx;
					if (childIdx & 2)
						nly = midy;
					else
						nuy = midy;
					if (childIdx & 1)
						nlz = midz;
					else
						nuz = midz;
					write_octree_points(fp, root->children[childIdx], nlx, nux, nly, nuy, nlz, nuz);
				}
			}
		}
	}
}


// Create an output file poiner based on the name of the input file
FILE* open_out_file(const char* const inFile, char* mode, char* fileExt)
{
	int numChars = 0;
	int lastSlashIdx = 0;

	for (; inFile[numChars] != '\0' && numChars < 50; numChars++)
	{
		if (inFile[numChars] == '\\' || inFile[numChars] == '/')
		{
			lastSlashIdx = numChars;
		}
	}

	char outFileName[80] = "../output/";
	int lenName = numChars - lastSlashIdx - 5 + 10;	// -5 to ignore the ".pcd", +10 to ignore the "..\\output\\"
	int charIdx = 10;
	for (; charIdx < lenName; charIdx++)
	{
		outFileName[charIdx] = inFile[++lastSlashIdx];
	}

	int extLen = strlen(fileExt);
	for (int extIdx = 0; extIdx <= extLen; extIdx++)
	{
		if (extIdx == extLen)	// Extra space to write null terminator. Inefficient but will be short so it doesn't really matter
		{
			outFileName[charIdx++] = '\0';
		}
		else
		{
			outFileName[charIdx++] = fileExt[extIdx];
		}
	}
	
	return fopen(outFileName, mode);
}
