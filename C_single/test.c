#include "single.h"
#include "miniunit.h"


// Test declarations
int test_tree_init();


int main(int argc, char* argv[])
{
	mu_run(test_tree_init);

	return 0;
}


// Test definitions
int test_tree_init()
{
	mu_start();

	OctreeNode* root = init_octree(0);
	for (int i = 0; i < 8; i++)
	{
		root->children[i] = init_octree(i * 8 + 1);
		for (int j = 0; j < 8; j++)
		{
			root->children[i]->children[j] = init_octree(i * 8 + j + 1);
		}
	}

	for (int i = 0; i < 8; i++)
	{
		mu_check(root->children[i]->data == i * 8 + 1);
		for (int j = 0; j < 8; j++)
		{
			mu_check(root->children[i]->children[j]->data == i * 8 + j + 1);
			for (int k = 0; k < 8; k++)
			{
				mu_check(root->children[i]->children[j]->children[k] == NULL);
			}
		}
	}

	delete_octree(root);

	mu_end();
}



