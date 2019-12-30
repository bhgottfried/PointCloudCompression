#include "octree.h"


// Call syntax: ./single [relative path to .pcd binary file to compress]
int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Incorrect number of arguments.\n");
		return EXIT_FAILURE; 
	}

	FILE* fp = fopen(argv[2], "rb");
	if (!fp)
	{
		printf("Failed to open file.\n");
		return EXIT_FAILURE;
	}

	// Read number of points from header
	unsigned int numPoints = parse_header(fp);
	if (!numPoints)
	{
		printf("Failed to parse .pcd header.\n");
		fclose(fp);
		return EXIT_FAILURE;
	}
	
	// Create array of 3D points
	Point** points = read_points(fp, numPoints);
	if (!points)
	{
		printf("Failed to parse point data.\n");
		fclose(fp);
		return EXIT_FAILURE;
	}

	// Find min and max values for each field
	float fieldMins[NUM_FIELDS];
	float fieldMaxs[NUM_FIELDS];
	get_min_max(points, numPoints, fieldMins, fieldMaxs);

	// Create octree for the read points
	OctreeNode* root = create_octree(points, numPoints, fieldMins, fieldMaxs);

	// TODO compress

	// Clean up dynamicaly allocated memory and files
	delete_octree(root);
	for (unsigned int ptIdx = 0; ptIdx < numPoints; ptIdx++)
	{
		free(points[ptIdx]);
	}
	free(points);
	fclose(fp);

	return EXIT_SUCCESS;
}
