#include "octree.h"


// Free dynamically allocated memory in PointSet
void delete_point_set(PointSet* ptSet)
{
	if (ptSet)
	{
		for (unsigned int ptIdx = 0; ptIdx < ptSet->numPoints; ptIdx++)
		{
			free(ptSet->points[ptIdx]);
		}
		free(ptSet->points);
		free(ptSet);
	}
}


// Free any dynamically allocated memory in the octree
void delete_octree(OctreeNode* root)
{
	if (root)
	{
		for (int i = 0; i < 8; i++)
		{
			delete_octree(root->children[i]);
		}
		free(root);
	}
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


// Create a deep copy of a byte list
ByteList* copy_byte_list(const ByteList* const bl)
{
	if (!bl || !bl->head)
	{
		return NULL;
	}

	ByteList* copy = malloc(sizeof(*copy));
	copy->numBytes = bl->numBytes;
	copy->head = malloc(sizeof(*copy->head));

	Link* currBl = bl->head;
	Link* currCopy = copy->head;

	for (int i = 0; i < bl->numBytes && currBl; i++)
	{
		currCopy->data = currBl->data;
		if (i < bl->numBytes - 1)
		{
			currCopy->next = malloc(sizeof(*currCopy->next));
			currCopy = currCopy->next;
			currBl = currBl->next;
		}
		else
		{
			currCopy->next = NULL;
			copy->tail = currCopy;
		}
	}

	return copy;
}


// Create a deep copy of an octree
OctreeNode* copy_octree(const OctreeNode* const tree)
{
	if (!tree)
	{
		return NULL;
	}

	OctreeNode* copy = malloc(sizeof(*copy));
	copy->data = tree->data;
	copy->isLeaf = tree->isLeaf;

	for (int i = 0; i < 8; i++)
	{
		copy->children[i] = copy_octree(tree->children[i]);
	}

	return copy;
}
