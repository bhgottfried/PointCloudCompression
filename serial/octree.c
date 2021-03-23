#include "octree.h"
#include "queue.h"


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
	
	else if (!(*root))
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


// Compare two octrees and return whether or not they are equal
bool are_equal(const OctreeNode* const A, const OctreeNode* const B)
{
	if (A && B)
	{
		if (A->data == B->data)
		{
			for (int i = 0; i < 8; i++)
			{
				if (!are_equal(A->children[i], B->children[i]))
				{
					return false;
				}
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return !A == !B;
	}
}


// Print the BFS for two trees
void print_trees(const OctreeNode* norm, const OctreeNode* reco)
{
	Queue* Q = init_queue();
	enqueue(Q, norm);
	enqueue(Q, reco);

	printf("  norm     reco  \n");
	printf("_________________\n");

	// Simultaneous breadth first traversal of both trees, appending difference byte for each visited node
	while (!is_empty(Q))
	{
		norm = dequeue(Q);
		reco = dequeue(Q);

		if (norm->isLeaf)
		{
			if (!reco->isLeaf)
				printf("reco missing leaf!\n");
			else
				printf("leaf    \tleaf\n");
		}
		else
		{
			for (int i = 0; i < 8; i++)
				printf("%d", norm->data & (1 << i) ? 1 : 0);
			printf(" ");
			for (int i = 0; i < 8; i++)
				printf("%d", reco->data & (1 << i) ? 1 : 0);

			if (norm->data != reco->data)
			{
				printf(" <-- Data mis-match!\n");
			}
			else
			{
				printf("\n");
				for (int i = 0; i < 8; i++)
				{
					if ((norm->data & (1 << i)) || (reco->data & (1 << i)))
					{
						enqueue(Q, norm->children[i]);
						enqueue(Q, reco->children[i]);
					}
				}
			}
		}
	}
	
	printf("\n");
	delete_queue(Q);
}
