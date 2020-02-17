#include "octree.h"
#include "queue.h"


// Breadth first traversal of the tree to write bytes for populated octants
ByteList* compress(const OctreeNode* const root)
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

	Queue* Q = init_queue();
	enqueue(Q, prevTree);
	
	while (!is_empty(Q))
	{
		
	}
	
	delete_queue(Q);
	return NULL;
}


// Remove dynamically allocated memory for ByteList by walking linked list
void delete_byte_list(ByteList* data)
{
	if (data)
	{
		Link* curr = data->head;
		Link* temp = NULL;

		while (curr)
		{
			temp = curr->next;
			free(curr);
			curr = temp;
		}
		free(data);
	}
}
