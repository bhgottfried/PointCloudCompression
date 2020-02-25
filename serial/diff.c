#include "octree.h"
#include "queue.h"


// XOR current tree with previous tree and return linked list of the difference
ByteList* calc_diff(const OctreeNode* curr, const OctreeNode* prev)
{
	ByteList* diff = malloc(sizeof(*diff));
	diff->head = malloc(sizeof(*diff->head));
	diff->numBytes = 0;
	Link* diffNode = diff->head;

	Queue* Q = init_queue();
	enqueue(Q, prev);
	enqueue(Q, curr);

	// Simultaneous breadth first traversal of both trees, appending difference byte for each visited node
	while (!is_empty(Q))
	{
		unsigned char diffByte;
		unsigned char childrenByte;
		prev = dequeue(Q);
		curr = dequeue(Q);

		// Don't diff leaves since the their data is 0
		if ((prev && prev->isLeaf) || (curr && curr->isLeaf))
		{
			continue;
		}
		
		if (prev && curr)	// Both the previous frame the current frame have data in this subvoxel
		{
			diffByte = prev->data ^ curr->data;
			childrenByte = prev->data | curr->data;
		}
		else	// There is a discrepancy between the last frame and the current frame in this subvoxel
		{
			if (prev)
			{
				diffByte = prev->data;
				childrenByte = prev->data;
			}
			else
			{
				diffByte = curr->data;
				childrenByte = curr->data;
			}
		}

		// Append the difference byte to the linked list
		if (diff->numBytes++)
		{
			diffNode->next = malloc(sizeof(*diffNode));
			diffNode = diffNode->next;
		}
		diffNode->data = diffByte;
		diffNode->next = NULL;
		diff->tail = diffNode;

		// Enqueue children for all subvoxels with data
		for (int childIdx = 0; childIdx < 8; childIdx++)
		{
			if (childrenByte & (1 << childIdx))
			{
				if (prev)
					enqueue(Q, prev->children[childIdx]);
				else
					enqueue(Q, NULL);
				
				if (curr)
					enqueue(Q, curr->children[childIdx]);
				else
					enqueue(Q, NULL);
			}
		}
	}

	delete_queue(Q);
	return diff;
}


// Construct current tree from previous tree plus diff
OctreeNode* reconstruct_from_diff(const OctreeNode* const prevTree, const ByteList* const diff)
{
	if (!prevTree || !diff)
	{
		return NULL;
	}

	OctreeNode* newRoot = init_node(false);
	Link* link = diff->head;
	Queue* Qprev = init_queue();
	Queue* Qnew  = init_queue();
	enqueue(Qprev, prevTree);
	enqueue(Qnew, newRoot);
	
	int currDepth = 0;
	int numNodesPrev = 1;
	int numNodesCurr = 1;
	
	while (link)
	{
		// Are we in a deeper level of the octree?
		if (!--numNodesPrev)
		{
			numNodesPrev = numNodesCurr;
			numNodesCurr = 0;
			currDepth++;
		}

		OctreeNode* currPrev = (OctreeNode*) dequeue(Qprev);
		OctreeNode* currNew  = (OctreeNode*) dequeue(Qnew);
		
		for (int childIdx = 0; childIdx < 8; childIdx++)
		{
			if ((link->data & (1 << childIdx)) || (currPrev && (currPrev->data & (1 << childIdx))))
			{
				numNodesCurr++;
				
				// If the previous tree had a child here, but there is not in the new tree
				if ((link->data & (1 << childIdx)) && currPrev && (currPrev->data & (1 << childIdx)))
				{
					if (currDepth < TARGET_DEPTH - 1)
					{
						enqueue(Qprev, currPrev->children[childIdx]);
						enqueue(Qnew, NULL);
					}
				}

				// Otherwise, the new tree has a node in this subvoxel, but the previous may or may not
				else
				{
					if (currNew)
					{
						if (currDepth == TARGET_DEPTH)
						{
							currNew->children[childIdx] = init_node(true);
							currNew->data |= 1 << childIdx;
						}
						else
						{
							currNew->children[childIdx] = init_node(false);
							enqueue(Qnew, currNew->children[childIdx]);
							currNew->data |= 1 << childIdx;

							if (link->data & (1 << childIdx))
							{
								// Previous tree was empty in this subvoxel, but the new tree is not
								enqueue(Qprev, NULL);
							}
							else
							{
								// Previous tree had data in this subvoxel, and so does the new
								enqueue(Qprev, currPrev->children[childIdx]);
							}	
						}
					}
				}
			}
		}

		link = link->next;
	}
	
	delete_queue(Qprev);
	delete_queue(Qnew);
	return newRoot;
}


// Merge two diff lists to get the difference from Ti to Tk
ByteList* merge_diff(const ByteList* const Dij, const ByteList* const Djk)
{
	if (!Dij || ! Djk)
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
	
	while (dataOld && dataNew && currOld && currNew)
	{
		unsigned char oByte = currOld->data | currNew->data;
		for (int bitIdx = 0; bitIdx < 8; bitIdx++)
		{
			// Interleave diff bytes for existing diff bytes for this subvoxel
			if (oByte & (1 << bitIdx))
			{
				unsigned char xByte = 0;
				if (currOld->data & (1 << bitIdx))
				{
					xByte ^= dataOld->data;
					dataOld = dataOld->next;
				}
				if (currNew->data & (1 << bitIdx))
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
		mergedLink->next = malloc(sizeof(*mergedLink));
		mergedLink = mergedLink->next;
		mergedLink->data = dataNew->data;
		mergedLink->next = NULL;
		dataNew = dataNew->next;
		sum->tail = mergedLink;
		sum->numBytes++;
	}

	return sum;
}
