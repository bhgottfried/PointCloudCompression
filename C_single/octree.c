#include "octree.h"


static void octree_insert(OctreeNode** root, OctreeNode* newNode, float lx, float ux, float ly, float uy, float lz, float uz);


// Create a new octree node for a point object.
static OctreeNode* init_node(Point* data)
{
	OctreeNode* node = malloc(sizeof(*node));
	node->data = data;
	for (int i = 0; i < 8; i++)
	{
		node->children[i] = NULL;
	}
	return node;
}


// Determine which suboctant to insert into
static void calc_bounds(OctreeNode** root, OctreeNode* newNode, float lx, float ux, float ly, float uy, float lz, float uz)
{
	int childIdx = 0;
	float midx = lx + (ux - lx) / 2, midy = ly + (uy - ly) / 2, midz = lz + (uz - lz) / 2;	// Midpoints of bounds

	// Compare the node data to the mid points
	if (newNode->data->coords[0] > midx)
	{
		lx = midx;
		childIdx += 4;
	}
	else
	{
		ux = midx;
	}
	if (newNode->data->coords[1] > midy)
	{
		ly = midy;
		childIdx += 2;
	}
	else
	{
		uy = midy;
	}
	if (newNode->data->coords[2] > midz)
	{
		lz = midz;
		childIdx += 1;
	}

	octree_insert(&(*root)->children[childIdx], newNode, lx, ux, ly, uy, lz, uz);
}


// Insert octree node into the tree recursively
static void octree_insert(OctreeNode** root, OctreeNode* newNode, float lx, float ux, float ly, float uy, float lz, float uz)
{
	if (!(*root))
	{
		*root = newNode;
	}
	else
	{
		if ((*root)->data)		// Former leaf node (root) now becomes internal node and leaf data gets reinserted
		{
			OctreeNode* prevNode = init_node((*root)->data);
			(*root)->data = NULL;
			calc_bounds(root, prevNode, lx, ux, ly, uy, lz, uz);
		}
		calc_bounds(root, newNode, lx, ux, ly, uy, lz, uz);
	}
}


// Generate full octree from point data
OctreeNode* create_octree(Point** points, unsigned int numPoints, float* fieldMins, float* fieldMaxs)
{
	OctreeNode* root = NULL;

	for (unsigned int ptIdx = 0; ptIdx < numPoints; ptIdx++)
	{
		OctreeNode* node = init_node(points[ptIdx]);
		octree_insert(&root, node, fieldMins[0], fieldMaxs[0], fieldMins[1], fieldMaxs[1], fieldMins[2], fieldMaxs[2]);
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
