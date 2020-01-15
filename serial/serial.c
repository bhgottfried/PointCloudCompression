#include "octree.h"


// Call syntax: ./single [relative path to .pcd binary file to compress]
int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Incorrect number of arguments.\n");
		return EXIT_FAILURE; 
	}

	FILE* inFilePtr = fopen(argv[1], "rb");
	if (!inFilePtr)
	{
		printf("Failed to open file.\n");
		return EXIT_FAILURE;
	}

	// Read number of points from header
	unsigned int numPoints = parse_header(inFilePtr);
	if (!numPoints)
	{
		printf("Failed to parse .pcd header.\n");
		fclose(inFilePtr);
		return EXIT_FAILURE;
	}
	
	// Create array of 3D points
	Point** points = read_points(inFilePtr, numPoints);
	if (!points)
	{
		printf("Failed to parse point data.\n");
		fclose(inFilePtr);
		return EXIT_FAILURE;
	}

	// Find min and max values for each field
	float fieldMins[NUM_FIELDS];
	float fieldMaxs[NUM_FIELDS];
	get_min_max(points, numPoints, fieldMins, fieldMaxs);

	// Create octree for the read points
	OctreeNode* root = create_octree(points, numPoints, fieldMins, fieldMaxs);

	// Open output file pointer
	FILE* outFilePtr = open_out_file(argv[1], "wb", ".pcdcmp");
	if (!outFilePtr)
	{
		printf("Failed to open the output file location.\n");
		delete_octree(root);
		for (unsigned int ptIdx = 0; ptIdx < numPoints; ptIdx++)
		{
			free(points[ptIdx]);
		}
		free(points);
		fclose(inFilePtr);
		return EXIT_FAILURE;
	}
	
	// Write min/max bounds to output file
	fwrite(fieldMins, FIELD_SIZE, NUM_FIELDS, outFilePtr);
	fwrite(fieldMaxs, FIELD_SIZE, NUM_FIELDS, outFilePtr);

	// Breadth first traversal of the tree to write bytes for populated octants
	compress(root, outFilePtr);	

	// Clean up dynamicaly allocated memory and files from compression
	delete_octree(root);
	for (unsigned int ptIdx = 0; ptIdx < numPoints; ptIdx++)
	{
		free(points[ptIdx]);
	}
	free(points);
	fclose(inFilePtr);
	fclose(outFilePtr);

	// Decompress newly compressed files for testing
	FILE* inCompFilePtr = open_out_file(argv[1], "rb", ".pcdcmp");
	FILE* outDcmpFilePtr = open_out_file(argv[1], "wb", ".pcdcmp.pcd");
	if (!inCompFilePtr || !outDcmpFilePtr)
	{
		printf("Failed to open files for decompression.\n");
		if (inCompFilePtr)
		{
			fclose(inCompFilePtr);
		}
		if (outDcmpFilePtr)
		{
			fclose(outDcmpFilePtr);
		}
		return EXIT_FAILURE;
	}

	// Decompress to test
	// decompress(inCompFilePtr, outDcmpFilePtr);

	// Close decompressed files
	fclose(inCompFilePtr);
	fclose(outFilePtr);

	return EXIT_SUCCESS;
}
