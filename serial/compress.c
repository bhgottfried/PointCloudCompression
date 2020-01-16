#include "octree.h"
#include "queue.h"


// Create an output file poiner based on the name of the input file
FILE* open_out_file(const char* const inFile, char* mode, char* fileExt)
{
	int numChars = 0;
	int lastSlashIdx = 0;

	for (; inFile[numChars] != '\0' && numChars < 50; numChars++)
	{
		if (inFile[numChars] == '\\' || inFile[numChars] == '/')
		{
			lastSlashIdx = numChars;
		}
	}

	char outFileName[80] = "../output/";
	int lenName = numChars - lastSlashIdx - 5 + 10;	// -5 to ignore the ".pcd", +10 to ignore the "..\\output\\"
	int charIdx = 10;
	for (; charIdx < lenName; charIdx++)
	{
		outFileName[charIdx] = inFile[++lastSlashIdx];
	}

	int extLen = strlen(fileExt);
	for (int extIdx = 0; extIdx <= extLen; extIdx++)
	{
		if (extIdx == extLen)	// Extra space to write null terminator. Inefficient but will be short so it doesn't really matter
		{
			outFileName[charIdx++] = '\0';
		}
		else
		{
			outFileName[charIdx++] = fileExt[extIdx];
		}
	}
	
	return fopen(outFileName, mode);
}


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


// Read in the compressed data, build an octree, and write the point set to a .pcd file
void decompress(FILE* const inFilePtr, FILE* const outFilePtr)
{
	float fieldMins[NUM_FIELDS];
	float fieldMaxs[NUM_FIELDS];
	fread(fieldMins, FIELD_SIZE, NUM_FIELDS, inFilePtr);
	fread(fieldMaxs, FIELD_SIZE, NUM_FIELDS, inFilePtr);

	OctreeNode* root = init_node(NULL);
	Queue* Q = init_queue();
	enqueue(Q, root);
	
	// Variables to handle counting number of points in compressed file
	char dataByte = 0;
	int numNodesPrev = 1;	// Set to 1 because of spaghetti, but it fixes off by one error...
	int numNodesCurr = 1;	// Set to 1 because of spaghetti, but it fixes off by one error...

	while ((dataByte = fgetc(inFilePtr)) != EOF)
	{
		// Are we in a deeper level of the octree?
		if (!--numNodesPrev)
		{
			numNodesPrev = numNodesCurr;
			numNodesCurr = 0;
		}

		OctreeNode* curr = (OctreeNode*) dequeue(Q);
		for (int childIdx = 0; childIdx < 8; childIdx++)
		{
			// If the bit is set in the compressed data byte, then that suboctant has a point in it
			if (dataByte & (1 << (7 - childIdx)))
			{
				curr->children[childIdx] = init_node(NULL);
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
