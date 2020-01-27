#include "octree.h"


// Create a new octree node for a point object.
OctreeNode* init_node(Point* data)
{
	OctreeNode* node = malloc(sizeof(*node));
	node->data = data;
	for (int i = 0; i < 8; i++)
	{
		node->children[i] = NULL;
	}
	return node;
}


// Insert octree node into the tree recursively
static void octree_insert(OctreeNode** root, Point* newPt, int depth, float lx, float ux, float ly, float uy, float lz, float uz)
{
	if (depth == TARGET_DEPTH)
	{
		if (*root == NULL)
		{
			*root = init_node(newPt);		// First point in this leaf suboctant
		}
		else
		{
			newPt->next = (*root)->data;	// Add point into linked list of points in this leaf suboctant
			(*root)->data = newPt;
		}
	}
	else	// Recursively find suboctant
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

		if (!(*root))
		{
			*root = init_node(NULL);	// First time visiting this suboctant
		}
		octree_insert(&(*root)->children[childIdx], newPt, depth + 1, lx, ux, ly, uy, lz, uz);
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


// Depth first traversal of octree, writing centroids of points when at leaf nodes
void write_octree_points(FILE* const fp, const OctreeNode* const root, int depth, float lx, float ux, float ly, float uy, float lz, float uz)
{
	if (root)
	{
		float midx = lx + (ux - lx) / 2, midy = ly + (uy - ly) / 2, midz = lz + (uz - lz) / 2;	// Midpoints of bounds
		if (depth < TARGET_DEPTH)
		{
			for (int childIdx = 0; childIdx < 8; childIdx++)
			{
				if (root->children[childIdx])
				{
					float nlx = lx, nux = ux, nly = ly, nuy = uy, nlz = lz, nuz = uz;	// Get new bounds for suboctant
					if (childIdx & 4)
						nlx = midx;
					else
						nux = midx;
					if (childIdx & 2)
						nly = midy;
					else
						nuy = midy;
					if (childIdx & 1)
						nlz = midz;
					else
						nuz = midz;
					write_octree_points(fp, root->children[childIdx], depth + 1, nlx, nux, nly, nuy, nlz, nuz);
				}
			}
		}
		else	// root is leaf node so write centroids to file
		{
			float centroid[] = { midx, midy, midz }; 
			fwrite(centroid, sizeof(*centroid), 3, fp);
		}
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
