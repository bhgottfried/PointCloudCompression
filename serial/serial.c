#include "octree.h"


// Call syntax: ./serial [-c for a single cloud [relative path to .pcd binary file to compress]
// Call syntax: ./serial [-s for stream] [num point clouds in stream] [paths to files to compress]
int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Call syntax: ./serial [-c for a single cloud [relative path to .pcd binary file to compress]\n");
		printf("Call syntax: ./serial [-s for stream] [num point clouds in stream] [paths to files to compress]\n");
		return EXIT_FAILURE; 
	}

	// Parse mode flag (single or stream)
	bool isStream;
	unsigned int numClouds = 1;
	if (!strcmp(argv[1], "-c"))
	{
		isStream = false;
	}
	else if (!strcmp(argv[1], "-s"))
	{
		isStream = true;
		numClouds = atoi(argv[2]);
	}
	else
	{
		printf("Call syntax: ./serial [-c for a single cloud [relative path to .pcd binary file to compress]\n");
		printf("Call syntax: ./serial [-s for stream] [num point clouds in stream] [paths to files to compress]\n");
		return EXIT_FAILURE; 
	}
	
	// Parse .pcd data and create set of points for inital cloud
	PointSet* P0 = get_point_set(argv[isStream ? 3 : 2]);
	if (!P0)
	{
		return EXIT_FAILURE;
	}

	// Create octree for the initial cloud
	OctreeNode* T0 = create_octree(P0);

	// Compress a single cloud
	if (!isStream)
	{
		// Open output file pointer
		FILE* outFilePtr = open_out_file(argv[2], "wb", ".pcdcmp");
		if (!outFilePtr)
		{
			printf("Failed to open the output file location.\n");
			delete_point_set(P0);
			delete_octree(T0);
			return EXIT_FAILURE;
		}
		
		// Write min/max bounds to output file
		fwrite(P0->mins, FIELD_SIZE, NUM_FIELDS, outFilePtr);
		fwrite(P0->maxs, FIELD_SIZE, NUM_FIELDS, outFilePtr);

		// Breadth first traversal of the tree to write bytes for populated octants
		compress(T0, outFilePtr);
		fclose(outFilePtr);

		// Decompress newly compressed files for testing
		FILE* inCompFilePtr = open_out_file(argv[2], "rb", ".pcdcmp");
		FILE* outDcmpFilePtr = open_out_file(argv[2], "wb", ".pcdcmp.pcd");
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
		decompress(inCompFilePtr, outDcmpFilePtr);

		// Close decompressed files
		fclose(inCompFilePtr);
		fclose(outDcmpFilePtr);
	}

	// Otherwise, compress the point cloud stream cloud by cloud
	else
	{
		// TODO
	}

	// Clean up dynamicaly allocated memory
	delete_point_set(P0);
	delete_octree(T0);

	return EXIT_SUCCESS;
}
