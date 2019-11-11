#include "single.h"

OctreeNode* init_octree(char data)
{
	OctreeNode* node = malloc(sizeof(*node));
	node->data = data;
	for (int i = 0; i < 8; i++)
	{
		node->children[i] = NULL;
	}
	return node;
}

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



