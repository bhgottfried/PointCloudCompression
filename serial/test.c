#include "octree.h"


// Static memory to be initialized at program start and deallocated after test is complete
static unsigned int numDiffs;
static OctreeNode** trees;
static ByteList** diffs;
static ByteList** merges;


// Initialize dynamic memory for testing purposes
void init_test(unsigned int _numDiffs, OctreeNode* T0)
{
	numDiffs = _numDiffs;
	trees = malloc((numDiffs + 1) * sizeof(*trees));
	diffs = malloc(numDiffs * sizeof(*diffs));
	merges = malloc(numDiffs * sizeof(*merges));
	trees[0] = T0;
}


// Test that the reconstruction from diff, merge, and prefix merge match the current tree
void test(OctreeNode* Ti, ByteList* Di, unsigned int i)
{
	trees[i + 1] = Ti;
	diffs[i] = Di;

	// Construct current tree from previous tree plus diff
	OctreeNode* testTree = reconstruct_from_diff(trees[i], diffs[i]);
	if (are_equal(testTree, Ti))
	{
		printf("reconstruct_from_diff %d success!\n", i);
	}
	else
	{
		printf("reconstruct_from_diff %d failure...\n", i);
	}
	delete_octree(testTree);

	// Merge the new diff with the previously merged diffs (the first merge is just the first diff)
	merges[i] = i ? merge_diff(merges[i - 1], diffs[i]) : diffs[i];
	
	// Ensure that the merge was successful
	testTree = reconstruct_from_diff(trees[0], merges[i]);
	if (are_equal(testTree, Ti))
	{
		printf("merge_diff %d success!\n", i);
	}
	else
	{
		printf("merge_diff %d failure...\n", i);
	}
	delete_octree(testTree);

	// Test construction of prefix merge
	OctreeNode** mergedTrees = prefix_merge(trees[0], diffs, i);
	for (int j = 0; j <= i; j++)
	{
		if (are_equal(mergedTrees[j], trees[j]))
		{
			printf("Prefix merge tree %d for first %d trees success!\n", j, i + 1);
		}
		else
		{
			printf("Prefix merge tree %d for first %d trees failure...\n", j, i + 1);
			break;
		}
	}

	// Remove memory for prefix merged trees
	for (int j = 0; j <= i; j++)
	{
		delete_octree(mergedTrees[i]);
	}
	free(mergedTrees);
}


// Clean up static memory after testing is complete
void clean_up_test(void)
{
	delete_octree(trees[0]);
	delete_octree(trees[1]);
	delete_byte_list(diffs[0]);
	for (int i = 1; i < numDiffs; i++)
	{
		delete_octree(trees[i + 1]);
		delete_byte_list(diffs[i]);
		delete_byte_list(merges[i]);
	}
	free(trees);
	free(diffs);
	free(merges);
}
