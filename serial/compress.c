#include "octree.h"
#include "queue.h"


// Breadth first traversal of the tree to write bytes for populated octants
ByteList* serialize(const OctreeNode* const root)
{
	if (!root)
	{
		return NULL;
	}

	ByteList* cloud = malloc(sizeof(*cloud));
	cloud->numBytes = 0;
	cloud->head = malloc(sizeof(*cloud->head));
	Link* link = cloud->head;

	Queue* Q = init_queue();
	enqueue(Q, root);
	
	while (!is_empty(Q))
	{
		OctreeNode* curr = (OctreeNode*) dequeue(Q);
		unsigned char val = 0;
		for (int childIdx = 0; childIdx < 8; childIdx++)
		{
			if (curr->children[childIdx])
			{
				val |= 1 << childIdx;
				if (!curr->children[childIdx]->isLeaf)
				{
					enqueue(Q, curr->children[childIdx]);
				}
			}
		}

		// Append the differnce byte to the linked list
		if (cloud->numBytes++)
		{
			link->next = malloc(sizeof(*link));
			link = link->next;
		}
		link->data = val;
		link->next = NULL;
		cloud->tail = link;
	}
	
	delete_queue(Q);
	return cloud;
}


// Decompress the data and build an octree
OctreeNode* decompress(FILE* const inFilePtr, float* fieldMins, float* fieldMaxs, int* numNodes)
{
	fread(fieldMins, FIELD_SIZE, NUM_FIELDS, inFilePtr);
	fread(fieldMaxs, FIELD_SIZE, NUM_FIELDS, inFilePtr);

	OctreeNode* root = init_node(false);
	Queue* Q = init_queue();
	enqueue(Q, root);
	
	// Variables to handle counting number of points in compressed file
	char dataByte = 0;
	int currDepth = 0;
	int numNodesPrev = 1;	// Set to 1 because of spaghetti, but it fixes off by one error...
	int numNodesCurr = 1;	// Set to 1 because of spaghetti, but it fixes off by one error...

	while ((dataByte = fgetc(inFilePtr)) != EOF)
	{
		// Are we in a deeper level of the octree?
		if (!--numNodesPrev)
		{
			numNodesPrev = numNodesCurr;
			numNodesCurr = 0;
			currDepth++;
		}

		OctreeNode* curr = (OctreeNode*) dequeue(Q);
		curr->data = dataByte;
		for (int childIdx = 0; childIdx < 8; childIdx++)
		{
			// If the bit is set in the compressed data byte, then that suboctant has a point in it
			if (dataByte & (1 << childIdx))
			{
				curr->children[childIdx] = init_node(currDepth == TARGET_DEPTH);
				enqueue(Q, curr->children[childIdx]);
				numNodesCurr++;
			}
		}
	}
	
	*numNodes = numNodesCurr;
	delete_queue(Q);
	return root;
}
