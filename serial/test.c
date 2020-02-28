#include "octree.h"


// Static memory to be initialized at program start and deallocated after test is complete
static const OctreeNode* T0;
static OctreeNode** trees;
static ByteList** diffs;
static ByteList** merges;
static unsigned int numDiffs;


// Initialize dynamic memory for testing purposes
void init_test(unsigned int _numDiffs, const OctreeNode* const _T0)
{
	numDiffs = _numDiffs;
	T0 = _T0;
	trees = malloc(numDiffs * sizeof(*trees));
	diffs = malloc(numDiffs * sizeof(*diffs));
	merges = malloc(numDiffs * sizeof(*merges));
}


// Test that the reconstruction from diff, merge, and prefix merge match the current tree
void test(OctreeNode* Ti, ByteList* Di, unsigned int i)
{
	trees[i] = Ti;
	diffs[i] = Di;

	// Construct current tree from previous tree plus diff
	OctreeNode* testTree = reconstruct_from_diff(i ? trees[i - 1] : T0, diffs[i]);
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
	testTree = reconstruct_from_diff(T0, merges[i]);
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
	OctreeNode** mergedTrees = prefix_merge(T0, diffs, i);
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
	delete_byte_list(diffs[0]);
	for (int i = 1; i < numDiffs; i++)
	{
		delete_byte_list(diffs[i]);
		delete_byte_list(merges[i]);
	}
	free(trees);
	free(diffs);
	free(merges);
}
