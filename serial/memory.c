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
