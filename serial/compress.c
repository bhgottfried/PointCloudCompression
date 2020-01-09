#include "octree.h"
#include "queue.h"


// Create an output file poiner based on the name of the input file
FILE* open_out_file(const char* const inFile)
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

	char outFileName[68] = "../output/";
	int lenName = numChars - lastSlashIdx - 5 + 10;	// -5 to ignore the ".pcd", +10 to ignore the "..\\output\\"
	int charIdx = 10;
	for (; charIdx < lenName; charIdx++)
	{
		outFileName[charIdx] = inFile[++lastSlashIdx];
	}

	char fileExt[8] = ".pcdcmp\0";
	for (int extIdx = 0; extIdx < 8; extIdx++)
	{
		outFileName[charIdx++] = fileExt[extIdx];
	}

	return fopen(outFileName, "wb");
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
