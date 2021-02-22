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

	int cnt = 0;
	
	while (dataOld && dataNew && currOld && currNew)
	{
		unsigned char oByte = currOld->data | currNew->data;
		for (int bitIdx = 0; bitIdx < 8; bitIdx++)
		{
			// Interleave diff bytes for existing diff bytes for this subvoxel
			if (oByte & (1 << bitIdx))
			{
				cnt++;

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
		cnt++;

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
		cnt++;

		mergedLink->next = malloc(sizeof(*mergedLink));
		mergedLink = mergedLink->next;
		mergedLink->data = dataNew->data;
		mergedLink->next = NULL;
		dataNew = dataNew->next;
		sum->tail = mergedLink;
		sum->numBytes++;
	}

	printf("cnt = %d\n", cnt);

	return sum;
}


// Thread timing struct
typedef struct _ThreadTimer
{
	double seq1;
	double merge[16];
	double seq2;
} ThreadTimer;


// Let n be the number of diffs to merge and let c be the number of processor cores.
// 1. Divide the array of diffs into c subarrays. Each core (simultaneously) sequentially merges n/c diffs.
//		(aside: if n < c then set c = n. If c does not divide n, then the cores with id < n mod c does 1 more diff than the others)
// 2. Now there c subarrays where the right most diff is the fully merged diff of its subarray. Perform parallel prefix merge
// 3. Then, each core sequentially merges each diff in its subarray, that is not the right most element, with the right most element of the previous subarray.
// 4. Finally, all trees are reconstructed in parallel with an initial tree and the ith merged diff.

// Perform parallel prefix-sum algorithm serially to join octree frames using initial tree and differences
OctreeNode** prefix_merge(const OctreeNode* const T0, ByteList** diffs, unsigned int numDiffs)
{
	ByteList** partial;
	ByteList* prev;
	int numThreads, busyOffset;
	int i, work, tid;

	// Temporary timing debugging variables
	ThreadTimer threadTimer, programTimer;
	int levelIdx = 0;

	ByteList** newMerges = malloc(numDiffs * sizeof(*newMerges));
	#pragma omp parallel for
	for (i = 0; i < numDiffs; i++)
	{
		newMerges[i] = copy_byte_list(diffs[i]);
	}

	// Clock variables
	double start = omp_get_wtime();
	double end, prevTime, currTime;
	// #pragma omp parallel default(none) private(i, tid, prev, work) shared(newMerges, partial, numThreads, numDiffs, busyOffset)
	#pragma omp parallel default(none) private(i, tid, prev, work, threadTimer) shared(newMerges, partial, numThreads, numDiffs, busyOffset, programTimer, levelIdx, start, end, prevTime, currTime)
	{
		#pragma omp single
		{
			numThreads = omp_get_num_threads();
			partial = malloc(sizeof(*partial) * numThreads);
			busyOffset =  numDiffs % numThreads;
		}

		tid = omp_get_thread_num();
		work = numDiffs / numThreads + (busyOffset > tid ? 1 : 0);

		// Timing for thread0 sequential merge 1
		if (!tid)
		{
			prevTime = omp_get_wtime();
		}

		// Compute the prefix sums for each thread until there are numThreads subarrays to merge
		for (i = work * tid + 1 + (tid < busyOffset ? 0 : busyOffset); i < work * tid + work + (tid < busyOffset ? 0 : busyOffset) && i < numDiffs; i++)
		{
			// printf("TID %d: Seq merge (%d,%d)\n", tid, i - 1, i);

			prev = newMerges[i];
			newMerges[i] = merge_diff(newMerges[i - 1], newMerges[i]);
			delete_byte_list(prev);

			if (!tid)
			{
				currTime = omp_get_wtime();
				printf("Thread0 seq1 merge time: %lf\n", currTime - prevTime);
				prevTime = currTime;
			}
		}

		if (i - 1 < numDiffs)
		{
			// printf("TID %d: partial = %d\n", tid, i - 1);
			partial[tid] = copy_byte_list(newMerges[i - 1]);
		}

		threadTimer.seq1 = omp_get_wtime() - start;
		
		#pragma omp barrier

		#pragma omp single
		{
			programTimer.seq1 = omp_get_wtime() - start;
		}
		
		// Calculate prefix merge for the array that was made from last elements of each of the previous sub-arrays
		for (i = 1; i < numThreads; i <<= 1)
		{
			if ((tid & i) && (tid < numDiffs))
			{
				// printf("TID %d: Prefix merge %d = (%d,%d)\n", tid, tid, (tid / i) * i - 1, tid);

				prev = partial[tid];
				partial[tid] = merge_diff(partial[(tid / i) * i - 1], partial[tid]);
				delete_byte_list(prev);
			}

			threadTimer.merge[levelIdx] = omp_get_wtime() - start;	// (levelIdx ? programTimer.merge[levelIdx - 1] : programTimer.seq1);

			#pragma omp barrier

			#pragma omp single
			{
				programTimer.merge[levelIdx] = omp_get_wtime() - start; // (levelIdx ? programTimer.merge[levelIdx - 1] : programTimer.seq1);
				levelIdx++;
				// printf("Moving to level %d\n", levelIdx);
			}
		}

		// Timing for thread0 sequential merge 2
		if (!tid)
		{
			prevTime = omp_get_wtime();
		}

		// Update each original thread's subarray

		for (i = work * tid + (tid < busyOffset ? 0 : busyOffset); i < min(work * tid + work + (tid < busyOffset ? 0 : busyOffset), numDiffs); i++)
		{
			// printf("TID %d: Seq merge for i = %d with partial\n", tid, i);

			prev = newMerges[i];
			newMerges[i] = merge_diff(newMerges[i], partial[tid]);
			delete_byte_list(prev);

			if (!tid)
			{
				currTime = omp_get_wtime();
				printf("Thread0 seq2 merge time: %lf\n", currTime - prevTime);
				prevTime = currTime;
			}
		}

		threadTimer.seq2 = omp_get_wtime() - start; // programTimer.merge[levelIdx - 1];
		
		#pragma omp barrier

		#pragma omp single
		{
			programTimer.seq2 = omp_get_wtime() - start; // programTimer.merge[levelIdx - 1];
		}

		#pragma omp single
		{
			end = omp_get_wtime();
		}

		// Timing check
		if (tid < numDiffs)
		{
			printf("TID %d: seq1 = %lf, \t merge0 = %lf, \t merge1 = %lf, \t merge2 = %lf, \t seq2 = %lf\n", tid, threadTimer.seq1, threadTimer.merge[0], threadTimer.merge[1], threadTimer.merge[2], threadTimer.seq2);
		}

		#pragma omp barrier

		#pragma omp single
		{
			printf("\nTotal: seq1 = %lf, \t merge0 = %lf, \t merge1 = %lf, \t merge2 = %lf, \t seq2 = %lf\n\n", programTimer.seq1, programTimer.merge[0], programTimer.merge[1], programTimer.merge[2], programTimer.seq2);
		}

	}

	OctreeNode** newTrees = malloc((numDiffs + 1) * sizeof(*newTrees));
	newTrees[0] = copy_octree(T0);

	#pragma omp parallel for
	for (int i = 1; i <= numDiffs; i++)
	{
		newTrees[i] = reconstruct_from_diff(T0, newMerges[i - 1]);
	}

	printf("Parallel prefix merge time for %d diffs: %.3lfs\n", numDiffs, end - start);

	for (i = 0; i < min(numDiffs, numThreads); i++)
	{
		delete_byte_list(partial[i]);
	}
	for (i = 0; i < numDiffs; i++)
	{
		delete_byte_list(newMerges[i]);
	}

	free(partial);
	free(newMerges);

	return newTrees;
}
