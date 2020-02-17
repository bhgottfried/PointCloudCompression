#include "octree.h"


// Create a new octree node for a point object.
OctreeNode* init_node(bool isLeaf)
{
	OctreeNode* node = malloc(sizeof(*node));
	node->isLeaf = isLeaf;
	node->data = 0;
	for (int i = 0; i < 8; i++)
	{
		node->children[i] = NULL;
	}
	return node;
}


// Insert octree node into the tree recursively
static void octree_insert(OctreeNode** root, Point* newPt, int depth, float lx, float ux, float ly, float uy, float lz, float uz)
{
	if (depth < TARGET_DEPTH)
	{
		int childIdx = 0;
		float midx = lx + (ux - lx) / 2, midy = ly + (uy - ly) / 2, midz = lz + (uz - lz) / 2;	// Midpoints of bounds

		// Compare the node data to the mid points
		if (newPt->coords[0] > midx)
		{
			lx = midx;
			childIdx += 4;
		}
		else
		{
			ux = midx;
		}
		if (newPt->coords[1] > midy)
		{
			ly = midy;
			childIdx += 2;
		}
		else
		{
			uy = midy;
		}
		if (newPt->coords[2] > midz)
		{
			lz = midz;
			childIdx += 1;
		}

		// First time visiting this suboctant
		if (*root == NULL)
		{
			*root = init_node(false);
		}

		(*root)->data |= 1 << childIdx;
		octree_insert(&(*root)->children[childIdx], newPt, depth + 1, lx, ux, ly, uy, lz, uz);
	}
	
	else
	{
		*root = init_node(true);
	}	
}


// Generate full octree from point data
OctreeNode* create_octree(const PointSet* const ptSet)
{
	OctreeNode* root = NULL;

	for (unsigned int ptIdx = 0; ptIdx < ptSet->numPoints; ptIdx++)
	{
		octree_insert(&root, ptSet->points[ptIdx], 0, ptSet->mins[0], ptSet->maxs[0], ptSet->mins[1], ptSet->maxs[1], ptSet->mins[2], ptSet->maxs[2]);
	}

	return root;
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
