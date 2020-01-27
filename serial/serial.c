#include "octree.h"


// Call syntax: ./serial [-c for a single cloud, or -s for stream] [relative path to .pcd binary file(s) to compress]
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("Incorrect number of arguments.\n");
		return EXIT_FAILURE; 
	}

	// Parse .pcd data and create set of points
	PointSet* ptSet = get_point_set(argv[2]);
	if (!ptSet)
	{
		return EXIT_FAILURE;
	}

	// Create octree for the read points
	OctreeNode* root = create_octree(ptSet);

	// Open output file pointer
	FILE* outFilePtr = open_out_file(argv[1], "wb", ".pcdcmp");
	if (!outFilePtr)
	{
		printf("Failed to open the output file location.\n");
		delete_point_set(ptSet);
		delete_octree(root);
		return EXIT_FAILURE;
	}
	
	// Write min/max bounds to output file
	fwrite(ptSet->mins, FIELD_SIZE, NUM_FIELDS, outFilePtr);
	fwrite(ptSet->maxs, FIELD_SIZE, NUM_FIELDS, outFilePtr);

	// Breadth first traversal of the tree to write bytes for populated octants
	compress(root, outFilePtr);	

	// Clean up dynamicaly allocated memory and files from compression
	delete_point_set(ptSet);
	delete_octree(root);
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
	decompress(inCompFilePtr, outDcmpFilePtr);

	// Close decompressed files
	fclose(inCompFilePtr);
	fclose(outDcmpFilePtr);

	return EXIT_SUCCESS;
}
