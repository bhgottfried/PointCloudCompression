#include "octree.h"


unsigned int TARGET_DEPTH = 0;


// Call syntax: ./serial [-c for a single cloud] [target depth] [relative path to .pcd binary file to compress]
// Call syntax: ./serial [-s for stream] [target depth] [num point clouds in stream] [path to compressed stream output] [paths to files to compress]
int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		printf("Call syntax: ./serial [-c for a single cloud] [target depth] [relative path to .pcd binary file to compress]\n");
		printf("Call syntax: ./serial [-s for stream] [target depth] [num point clouds in stream] [path to compressed stream output] [paths to files to compress]\n");
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
		numClouds = atoi(argv[3]);
		if (numClouds == 0)
		{
			printf("Number of clouds must be a positive integer.\n");
			return EXIT_FAILURE;
		}
	}
	else
	{
		printf("Call syntax: ./serial [-c for a single cloud] [target depth] [relative path to .pcd binary file to compress]\n");
		printf("Call syntax: ./serial [-s for stream] [target depth] [num point clouds in stream] [path to compressed stream output] [paths to files to compress]\n");
		return EXIT_FAILURE;
	}

	// Parse target depth
	TARGET_DEPTH = (unsigned int) atoi(argv[2]);
	if (TARGET_DEPTH == 0)
	{
		printf("Depth must be a positive integer.\n");
		return EXIT_FAILURE;
	}
	
	// Parse .pcd data and create set of points for inital cloud
	PointSet* P0 = get_point_set(argv[isStream ? STREAM_PATH_0_IDX : SINGLE_PATH_IDX]);
	if (!P0)
	{
		return EXIT_FAILURE;
	}

	// Create octree for the initial cloud
	OctreeNode* T0 = create_octree(P0);

	// Breadth first traversal of the tree to write bytes for populated octants
	ByteList* serialization = compress(T0);
	if (!serialization)
	{
		printf("Failed to compress tree");
		return EXIT_FAILURE;
	}

	// Compress a single cloud
	if (!isStream)
	{
		// Open output file pointer
		FILE* fp = open_out_file(argv[SINGLE_PATH_IDX], "wb", ".pcdcmp");
		if (!fp)
		{
			printf("Failed to open the output file location.\n");
			delete_point_set(P0);
			delete_octree(T0);
			return EXIT_FAILURE;
		}
		
		// Write min/max bounds to output file
		fwrite(P0->mins, FIELD_SIZE, NUM_FIELDS, fp);
		fwrite(P0->maxs, FIELD_SIZE, NUM_FIELDS, fp);

		// Write serialized tree to file
		write_byte_list(serialization, fp);
		delete_byte_list(serialization);
		fclose(fp);

		// Decompress the newly written file to test
		float fieldMins[NUM_FIELDS];
		float fieldMaxs[NUM_FIELDS];
		unsigned int numNodes = 0;
		fp = open_out_file(argv[SINGLE_PATH_IDX], "rb", ".pcdcmp");
		if (!fp)
		{
			printf("Failed to open files for decompression.\n");
			return EXIT_FAILURE;
		}
		OctreeNode* root = decompress(fp, fieldMins, fieldMaxs, &numNodes);
		fclose(fp);

		// Write the decompressed centroids to a new .pcd file
		fp = open_out_file(argv[SINGLE_PATH_IDX], "wb", ".pcdcmp.pcd");
		if (!fp)
		{
			printf("Failed to open output file location for centroid writing.\n");
			delete_octree(root);
			return EXIT_FAILURE;
		}
		write_pcd_header(fp, numNodes);
		write_octree_points(fp, root, fieldMins[0], fieldMaxs[0], fieldMins[1], fieldMaxs[1], fieldMins[2], fieldMaxs[2]);

		// Close decompressed files and free memory
		delete_octree(root);
		fclose(fp);
	}

	// Otherwise, compress the point cloud stream cloud by cloud
	else
	{
		// Write header to output stream and validate file pointer
		FILE* fp = write_stream_header(argv[4], numClouds);
		if (!fp)
		{
			delete_point_set(P0);
			delete_octree(T0);
			delete_byte_list(serialization);
			return EXIT_FAILURE;
		}

		// Write the initial tree with bounds to the output stream
		fwrite(&(serialization->numBytes), sizeof(serialization->numBytes), 1, fp);
		fwrite(P0->mins, FIELD_SIZE, NUM_FIELDS, fp);
		fwrite(P0->maxs, FIELD_SIZE, NUM_FIELDS, fp);
		write_byte_list(serialization, fp);

		PointSet* prevPtSet  = P0;
		PointSet* currPtSet  = NULL;
		OctreeNode* prevTree = T0;
		OctreeNode* currTree = NULL;

		for (unsigned int cloudIdx = 1; cloudIdx < numClouds; cloudIdx++)
		{
			// Parse next point data and create octree
			currPtSet = get_point_set(argv[STREAM_PATH_0_IDX + cloudIdx]);
			if (!currPtSet)
			{
				printf("Invalid .pcd index (Initial .pcd = 0): %u.\n", cloudIdx);
				delete_point_set(prevPtSet);
				delete_octree(prevTree);
				delete_point_set(P0);
				delete_octree(T0);
				return EXIT_FAILURE;
			}
			currTree = create_octree(currPtSet);

			// Calc bitwise difference between last tree and current tree
			ByteList* diff = calc_diff(currTree, prevTree);
			
			// Write new bounds of tree followed by diff from previous frame to new.
			fwrite(&(diff->numBytes), sizeof(diff->numBytes), 1, fp);
			fwrite(currPtSet->mins, FIELD_SIZE, NUM_FIELDS, fp);
			fwrite(currPtSet->maxs, FIELD_SIZE, NUM_FIELDS, fp);
			write_byte_list(diff, fp);

			// Construct current tree from previous tree plus diff
			OctreeNode* testTree = reconstruct_from_diff(prevTree, diff);
			if (are_equal(testTree, currTree))
			{
				printf("Trees equal!\n");
			}
			else
			{
				printf("Not equal...\n");
			}
			delete_octree(testTree);

			// Free dynamically allocated memory for trees, except the initial tree
			if (cloudIdx > 1)
			{
				delete_point_set(prevPtSet);
				delete_octree(prevTree);
			}
			delete_byte_list(diff);
			prevPtSet = currPtSet;
			prevTree = currTree;
		}

		// Delete memory for the last tree in the stream
		delete_point_set(prevPtSet);
		delete_octree(prevTree);
		fclose(fp);
	}

	// Clean up dynamicaly allocated memory
	delete_point_set(P0);
	delete_octree(T0);

	return EXIT_SUCCESS;
}
