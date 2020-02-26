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
	ByteList* serialization = serialize(T0);
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

		// Create array to hold diff lists for each new frame (for testing)
		// When not testing, use a single diff pointer and don't store merges
		int numDiffs = numClouds - 1;
		ByteList** diffs = malloc(numDiffs * sizeof(*diffs));
		ByteList** merges = malloc(numDiffs * sizeof(*merges));

		// Write the initial tree with bounds to the output stream
		fwrite(&(serialization->numBytes), sizeof(serialization->numBytes), 1, fp);
		fwrite(P0->mins, FIELD_SIZE, NUM_FIELDS, fp);
		fwrite(P0->maxs, FIELD_SIZE, NUM_FIELDS, fp);
		write_byte_list(serialization, fp);

		PointSet* prevPtSet  = P0;
		PointSet* currPtSet  = NULL;
		OctreeNode* prevTree = T0;
		OctreeNode* currTree = NULL;

		for (unsigned int diffIdx = 0; diffIdx < numDiffs; diffIdx++)
		{
			// Parse next point data and create octree
			currPtSet = get_point_set(argv[STREAM_PATH_0_IDX + diffIdx + 1]);
			if (!currPtSet)
			{
				printf("Invalid .pcd index (Initial .pcd = 0): %u.\n", diffIdx + 1);
				delete_point_set(prevPtSet);
				delete_octree(prevTree);
				delete_point_set(P0);
				delete_octree(T0);
				return EXIT_FAILURE;
			}
			currTree = create_octree(currPtSet);

			// Calc bitwise difference between last tree and current tree
			diffs[diffIdx] = calc_diff(currTree, prevTree);
			
			// Write new bounds of tree followed by diff from previous frame to new.
			fwrite(&(diffs[diffIdx]->numBytes), sizeof(diffs[diffIdx]->numBytes), 1, fp);
			fwrite(currPtSet->mins, FIELD_SIZE, NUM_FIELDS, fp);
			fwrite(currPtSet->maxs, FIELD_SIZE, NUM_FIELDS, fp);
			write_byte_list(diffs[diffIdx], fp);

			// ------------------------------ BEGIN TEST ONLY SECTION ------------------------------
			// Construct current tree from previous tree plus diff
			OctreeNode* testTree = reconstruct_from_diff(prevTree, diffs[diffIdx]);
			if (are_equal(testTree, currTree))
			{
				printf("reconstruct_from_diff %d success!\n", diffIdx);
			}
			else
			{
				printf("reconstruct_from_diff %d failure...\n", diffIdx);
			}
			delete_octree(testTree);

			// Merge the new diff with the previously merged diffs
			if (diffIdx)
			{
				merges[diffIdx] = merge_diff(merges[diffIdx - 1], diffs[diffIdx]);
			}
			else
			{
				merges[diffIdx] = diffs[diffIdx];
			}
			
			// Ensure that the merge was successful
			testTree = reconstruct_from_diff(T0, merges[diffIdx]);
			if (are_equal(testTree, currTree))
			{
				printf("merge_diff %d success!\n", diffIdx);
			}
			else
			{
				printf("merge_diff %d failure...\n", diffIdx);
			}
			delete_octree(testTree);

			// Serial prefix sum and test memory cleanup after final tree
			if (diffIdx == numDiffs - 1)
			{
				int currNum = numDiffs;
				ByteList** currMerges = malloc(currNum * sizeof(*currMerges));
				for (int i = 0; i < currNum; i++)
				{
					currMerges[i] = copy_byte_list(diffs[i]);
				}

				while (currNum > 1)
				{
					int nextNum = currNum / 2 + currNum % 2;
					ByteList** nextMerges = malloc(nextNum * sizeof(*nextMerges));
					for (int i = 0; i < nextNum; i++)
					{
						if (i < nextNum / 2)
						{
							nextMerges[i] = merge_diff(currMerges[2 * i], currMerges[2 * i + 1]);
						}
						else	// If curr num is odd, there will be a loner to not get merged
						{
							nextMerges[i] = currMerges[currNum - 1];
						}
					}

					// Remove old diffs/merges that are no longer needed
					for (int i = 0; i < currNum; i++)
					{
						delete_byte_list(currMerges[i]);
					}
					free(currMerges);
					
					currNum = nextNum;
					currMerges = nextMerges;
				}

				// Test construction of prefix merge
				testTree = reconstruct_from_diff(T0, *currMerges);
				if (are_equal(testTree, currTree))
				{
					printf("Prefix merge success!\n");
				}
				else
				{
					printf("Prefix merge failure...\n");
				}

				// Delete prefix memory
				delete_octree(testTree);
				delete_byte_list(*currMerges);
				free(currMerges);

				// Delete list of diffs and merges
				delete_byte_list(diffs[0]);
				for (int i = 1; i < numDiffs; i++)
				{
					delete_byte_list(diffs[i]);
					delete_byte_list(merges[i]);
				}
				free(merges);
				free(diffs);
			}
			// ------------------------------ END TEST ONLY SECTION ------------------------------

			// Free dynamically allocated memory for trees, except the initial tree
			if (diffIdx)
			{
				delete_point_set(prevPtSet);
				delete_octree(prevTree);
			}
			prevPtSet = currPtSet;
			prevTree = currTree;
		}

		// Delete memory for the last tree in the stream
		delete_point_set(prevPtSet);
		delete_octree(prevTree);
		fclose(fp);
	}

	// Clean up dynamicaly allocated memory
	delete_byte_list(serialization);
	delete_point_set(P0);
	delete_octree(T0);

	return EXIT_SUCCESS;
}
