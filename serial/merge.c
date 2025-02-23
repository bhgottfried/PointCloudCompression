#include "octree.h"
#include <time.h>


// Merge two diff lists to get the difference from Ti to Tk
ByteList* merge_diff(const ByteList* const Dij, const ByteList* const Djk)
{
	if (!Dij || !Djk)
	{
		printf("Invalid arguments to merge_diff.\n");
		return NULL;
	}

	printf("Dij len = %d\n", Dij->numBytes);
	printf("Djk len = %d\n", Djk->numBytes);

	// Initialize resulting, merged list
	ByteList* sum = malloc(sizeof(*sum));
	sum->head = malloc(sizeof(*sum->head));
	sum->numBytes = 0;

	// Current links for all linked lists
	Link* mergedLink = sum->head;
	Link* currOld = Dij->head;
	Link* currNew = Djk->head;
	Link* dataOld = currOld;
	Link* dataNew = currNew;

	// Append the XORed heads to the result
	sum->numBytes++;
	mergedLink->data = currOld->data ^ currNew->data;
	mergedLink->next = NULL;
	sum->tail = mergedLink;
	
	int a = 0;
	int b = 0;
	int c = 0;
	
	while (dataOld && dataNew && currOld && currNew)
	{
		unsigned char oByte = currOld->data | currNew->data;
		for (int bitIdx = 0; bitIdx < 8; bitIdx++)
		{
			// Interleave diff bytes for existing diff bytes for this subvoxel
			if (oByte & (1 << bitIdx))
			{
				a++;

				unsigned char xByte = 0;
				if ((currOld->data & (1 << bitIdx)) && dataOld)
				{
					xByte ^= dataOld->data;
					dataOld = dataOld->next;
				}
				if ((currNew->data & (1 << bitIdx)) && dataNew)
				{
					xByte ^= dataNew->data;
					dataNew = dataNew->next;
				}

				// Append the merged difference byte to the linked list
				mergedLink->next = malloc(sizeof(*mergedLink));
				mergedLink = mergedLink->next;
				mergedLink->data = xByte;
				mergedLink->next = NULL;
				sum->tail = mergedLink;
				sum->numBytes++;
			}
		}

		// Proceed to the next subtree
		currOld = currOld->next;
		currNew = currNew->next;
	}

	// Append any remaining bytes that do not need to be interleaved
	while (dataOld)
	{
		b++;

		mergedLink->next = malloc(sizeof(*mergedLink));
		mergedLink = mergedLink->next;
		mergedLink->data = dataOld->data;
		mergedLink->next = NULL;
		dataOld = dataOld->next;
		sum->tail = mergedLink;
		sum->numBytes++;
	}
	while (dataNew)
	{
		c++;

		mergedLink->next = malloc(sizeof(*mergedLink));
		mergedLink = mergedLink->next;
		mergedLink->data = dataNew->data;
		mergedLink->next = NULL;
		dataNew = dataNew->next;
		sum->tail = mergedLink;
		sum->numBytes++;
	}

	printf("a = %d \t b = %d  \t c = %d\n", a,b,c);

	return sum;
}


// Perform parallel prefix-sum algorithm serially to join octree frames using initial tree and differences
OctreeNode** prefix_merge(const OctreeNode* const T0, ByteList** diffs, unsigned int numDiffs)
{
	ByteList** newMerges = malloc(numDiffs * sizeof(*newMerges));
	for (int i = 0; i < numDiffs; i++)
	{
		newMerges[i] = copy_byte_list((diffs[i]));
	}

	// for (int stride = 1; stride < numDiffs; stride *= 2)
	// {
	// 	for (int i = stride; i < numDiffs; i += stride * 2)
	// 	{
	// 		for (int j = 0; j < stride && i + j < numDiffs; j++)
	// 		{
	// 			ByteList* prev = newMerges[i + j];
	// 			newMerges[i + j] = merge_diff(newMerges[i - 1], newMerges[i + j]);
	// 			delete_byte_list(prev);
	// 		}
	// 	}
	// }
	
	clock_t start = clock();
	clock_t last = start;
	
	for (int i = 1; i < numDiffs; i++)
	{
		ByteList* prev = newMerges[i];
		newMerges[i] = merge_diff(newMerges[i - 1], newMerges[i]);
		delete_byte_list(prev);

		// // Timing check
		// clock_t next = clock();
		// printf("%lf\n", (double) (next - last) / CLOCKS_PER_SEC);
		// last = next;
	}

	OctreeNode** newTrees = malloc((numDiffs + 1)* sizeof(*newTrees));
	newTrees[0] = copy_octree(T0);
	for (int i = 1; i <= numDiffs; i++)
	{
		newTrees[i] = reconstruct_from_diff(T0, newMerges[i - 1]);
	}

	clock_t end = clock();
	double time = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("Serial prefix merge time for %d diffs: %.3lfs\n", numDiffs, time);

	for (int i = 1; i <= numDiffs; i++)
	{
		delete_byte_list(newMerges[i - 1]);
	}
	free(newMerges);

	return newTrees;
}
