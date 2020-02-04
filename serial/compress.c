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
			if (curr->children[childIdx])
			{
				if (!curr->children[childIdx]->data)	// A node has children iff. data == NULL
				{
					enqueue(Q, curr->children[childIdx]);
				}
				val |= 1 << (7 - childIdx);
			}
		}
		fputc(val, fp);
	}
	
	delete_queue(Q);
}


// TODO decompress is currently not working, but should be an easy fix... later
// Read in the compressed data, build an octree, and write the point set to a .pcd file
void decompress(FILE* const inFilePtr, FILE* const outFilePtr)
{
	float fieldMins[NUM_FIELDS];
	float fieldMaxs[NUM_FIELDS];
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
		for (int childIdx = 0; childIdx < 8; childIdx++)
		{
			// If the bit is set in the compressed data byte, then that suboctant has a point in it
			if (dataByte & (1 << (7 - childIdx)))
			{
				curr->children[childIdx] = init_node(currDepth == TARGET_DEPTH);
				enqueue(Q, curr->children[childIdx]);
				numNodesCurr++;
			}
		}
	}
	
	delete_queue(Q);
	write_header(outFilePtr, numNodesCurr);
	write_octree_points(outFilePtr, root, 0, fieldMins[0], fieldMaxs[0], fieldMins[1], fieldMaxs[1], fieldMins[2], fieldMaxs[2]);
	delete_octree(root);
}


// XOR current tree with previous tree and return linked list of the difference
DiffDataLL* calc_diff(const OctreeNode* const curr, const OctreeNode* const prev)
{
	// TODO
	return NULL;
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
