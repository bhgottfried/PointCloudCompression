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
	PointSet* P0 = get_point_set(argv[isStream ? PATH_0_IDX : 2]);
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
		FILE* fp = open_out_file(argv[2], "wb", ".pcdcmp");
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

		// Breadth first traversal of the tree to write bytes for populated octants
		compress(T0, fp);
		fclose(fp);

		// Decompress the newly written file to test
		float fieldMins[NUM_FIELDS];
		float fieldMaxs[NUM_FIELDS];
		unsigned int numNodes = 0;
		fp = open_out_file(argv[2], "rb", ".pcdcmp");
		if (!fp)
		{
			printf("Failed to open files for decompression.\n");
			return EXIT_FAILURE;
		}
		OctreeNode* root = decompress(fp, fieldMins, fieldMaxs, &numNodes);
		fclose(fp);

		// Write the decompressed centroids to a new .pcd file
		fp = open_out_file(argv[2], "wb", ".pcdcmp.pcd");
		if (!fp)
		{
			printf("Failed to open output file location for centroid writing.\n");
			delete_octree(root);
			return EXIT_FAILURE;
		}
		write_header(fp, numNodes);
		write_octree_points(fp, root, 0, fieldMins[0], fieldMaxs[0], fieldMins[1], fieldMaxs[1], fieldMins[2], fieldMaxs[2]);

		// Close decompressed files and free memory
		delete_octree(root);
		fclose(fp);
	}

	// Otherwise, compress the point cloud stream cloud by cloud
	else
	{
		PointSet* prevPtSet  = P0;
		PointSet* currPtSet  = NULL;
		OctreeNode* prevTree = T0;
		OctreeNode* currTree = NULL;

		for (unsigned int cloudIdx = 1; cloudIdx < numClouds; cloudIdx++)
		{
			// Parse next point data and create octree
			currPtSet = get_point_set(argv[PATH_0_IDX + cloudIdx]);
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
			DiffDataLL* diff = calc_diff(currTree, prevTree);
			

			// TODO do something with the diff data.


			// Free dynamically allocated memory for trees, except the initial tree
			if (cloudIdx > 1)
			{
				delete_point_set(prevPtSet);
				delete_octree(prevTree);
			}
			delete_diff_data(diff);
			prevPtSet = currPtSet;
			prevTree = currTree;
		}

		// Delete memory for the last tree in the stream
		delete_point_set(prevPtSet);
		delete_octree(prevTree);
	}

	// Clean up dynamicaly allocated memory
	delete_point_set(P0);
	delete_octree(T0);

	return EXIT_SUCCESS;
}
