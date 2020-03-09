#include "octree.h"


unsigned int TARGET_DEPTH = 0;


// Call syntax: ./parallel [target depth] [num point clouds in stream] [path to compressed stream output] [paths to files to compress]
int main(int argc, char* argv[])
{
	if (argc < 6)
	{
		printf("Call syntax: ./parallel [target depth] [num point clouds in stream] [path to compressed stream output] [paths to files to compress]\n");
		return EXIT_FAILURE; 
	}

	// Parse number of clouds and target depth
	TARGET_DEPTH = (unsigned int) atoi(argv[2]);
	unsigned int numClouds = (unsigned int) atoi(argv[3]);
	if (numClouds == 0 || TARGET_DEPTH == 0)
	{
		printf("Depth and number of clouds must be a positive integer.\n");
		return EXIT_FAILURE;
	}
	
	// Parse .pcd data and create set of points for inital cloud
	PointSet* P0 = get_point_set(argv[STREAM_PATH_0_IDX]);
	if (!P0)
	{
		return EXIT_FAILURE;
	}

	// Create octree for the initial cloud
	OctreeNode* T0 = create_octree(P0);

	// Breadth first traversal of the tree to write bytes for populated octants
	ByteList* serialization = serialize(T0);
	if (!serialization)
	{
		printf("Failed to compress tree");
		return EXIT_FAILURE;
	}
	
	// Write header to output stream and validate file pointer
	FILE* fp = write_stream_header(argv[OUTPUT_PATH_IDX], numClouds);
	if (!fp)
	{
		delete_point_set(P0);
		delete_octree(T0);
		delete_byte_list(serialization);
		return EXIT_FAILURE;
	}

	// Initialize values for stream compression calculations and test
	int numDiffs = numClouds - 1;
	init_test(numDiffs, copy_octree(T0));

	// Write the initial tree with bounds to the output stream
	fwrite(&(serialization->numBytes), sizeof(serialization->numBytes), 1, fp);
	fwrite(P0->mins, FIELD_SIZE, NUM_FIELDS, fp);
	fwrite(P0->maxs, FIELD_SIZE, NUM_FIELDS, fp);
	write_byte_list(serialization, fp);
	delete_byte_list(serialization);

	PointSet* prevPtSet  = P0;
	PointSet* currPtSet  = NULL;
	OctreeNode* prevTree = T0;
	OctreeNode* currTree = NULL;

	// Compress the point cloud stream cloud by cloud
	for (unsigned int diffIdx = 0; diffIdx < numDiffs; diffIdx++)
	{
		// Parse next point data and create octree
		currPtSet = get_point_set(argv[STREAM_PATH_0_IDX + diffIdx + 1]);
		if (!currPtSet)
		{
			printf("Invalid .pcd index (Initial .pcd = 0): %u.\n", diffIdx + 1);
			delete_point_set(prevPtSet);
			delete_octree(prevTree);
			clean_up_test(diffIdx);
			fclose(fp);
			return EXIT_FAILURE;
		}
		currTree = create_octree(currPtSet);

		// Calc bitwise difference between last tree and current tree
		ByteList* currDiff = calc_diff(currTree, prevTree);
		
		// Write new bounds of tree followed by diff from previous frame to new.
		fwrite(&(currDiff->numBytes), sizeof(currDiff->numBytes), 1, fp);
		fwrite(currPtSet->mins, FIELD_SIZE, NUM_FIELDS, fp);
		fwrite(currPtSet->maxs, FIELD_SIZE, NUM_FIELDS, fp);
		write_byte_list(currDiff, fp);

		// Test that the reconstruction from diff, merge, and prefix merge match the current tree
		test(copy_octree(currTree), copy_byte_list(currDiff), diffIdx);

		// Delete memory for the current diff
		delete_byte_list(currDiff);

		// Free dynamically allocated memory for the previous tree
		delete_point_set(prevPtSet);
		delete_octree(prevTree);
		prevPtSet = currPtSet;
		prevTree = currTree;
	}

	// Delete memory for the last tree in the stream
	delete_point_set(prevPtSet);
	delete_octree(prevTree);
	fclose(fp);

	// Clean up dynamicaly allocated memory for testing
	clean_up_test(numClouds - 1);

	return EXIT_SUCCESS;
}
