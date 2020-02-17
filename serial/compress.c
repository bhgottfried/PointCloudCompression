#include "octree.h"
#include "queue.h"


// Breadth first traversal of the tree to write bytes for populated octants
void compress(const OctreeNode* const root, FILE* const fp)
{
	if (!root)
	{
		return;
	}

	Queue* Q = init_queue();
	enqueue(Q, root);
	
	while (!is_empty(Q))
	{
		OctreeNode* curr = (OctreeNode*) dequeue(Q);
		unsigned char val = 0;
		for (int childIdx = 0; childIdx < 8; childIdx++)
		{
			if (curr->children[childIdx] && !curr->children[childIdx]->isLeaf)
			{
				enqueue(Q, curr->children[childIdx]);
			}
		}
		fputc(curr->data, fp);
	}
	
	delete_queue(Q);
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
DiffDataLL* calc_diff(const OctreeNode* curr, const OctreeNode* prev)
{
	DiffDataLL* diffList = malloc(sizeof(*diffList));
	DiffDataLL* diffNode = diffList;
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

		// Append the differnce byte to the resulting linked list
		diffNode->data = diffByte;
		diffNode->next = malloc(sizeof(*(diffNode->next)));
		diffNode = diffNode->next;

		// Enqueue children for all subvoxels with data
		for (int childIdx = 0; childIdx < 8; childIdx++)
		{
			if (childrenByte & (1 << childIdx))
			{
				if (prev)
				{
					enqueue(Q, prev->children[childIdx]);
				}
				else
				{
					enqueue(Q, NULL);
				}
				
				if (curr)
				{
					enqueue(Q, curr->children[childIdx]);
				}
				else
				{
					enqueue(Q, NULL);
				}
			}
		}
	}

	delete_queue(Q);
	return diffList;
}


// Remove dynamically allocated memory for DiffDataLL by walking linked list
void delete_diff_data(DiffDataLL* list)
{
	DiffDataLL* curr = list;
	DiffDataLL* temp = NULL;

	while (curr)
	{
		temp = curr->next;
		free(curr);
		curr = temp;
	}
}
