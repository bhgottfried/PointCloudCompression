#ifndef __OCTREE_H_
#define __OCTREE_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>


#define TARGET_DEPTH 3	// Can be added into the input file header, but will violate the .pcd standard
#define NUM_FIELDS 3
#define FIELD_SIZE (sizeof(float))
#define PATH_0_IDX 3	// Index in argv that contains the path to the first tree in the stream


typedef struct _Point
{
	struct _Point* next;
	float coords[NUM_FIELDS];
} Point;

typedef struct _PointSet
{
	unsigned int numPoints;
	float mins[NUM_FIELDS];
	float maxs[NUM_FIELDS];
	Point** points;
} PointSet;

typedef struct _OctreeNode
{
	bool isLeaf;
	unsigned char data;
	struct _OctreeNode* children[8];
} OctreeNode;

typedef struct _DiffDataLL
{
	unsigned char difference;
	struct _DiffDataLL* next;
} DiffDataLL;


// Point functions
PointSet* get_point_set(const char* const fileName);
void delete_point_set(PointSet* ptSet);
// Octree functions
OctreeNode* init_node(bool isLeaf);
OctreeNode* create_octree(const PointSet* const ptSet);
DiffDataLL* calc_diff(OctreeNode* curr, OctreeNode* prev);
void delete_octree(OctreeNode* root);
// Compression functions
void compress(const OctreeNode* const root, FILE* const fp);
void decompress(FILE* const inFilePtr, FILE* const outFilePtr);
void delete_diff_data(DiffDataLL* list);
// Public FileIO functions
void write_header(FILE* const fp, unsigned int numPoints);
void write_octree_points(FILE* const fp, const OctreeNode* const root, int depth, float lx, float ux, float ly, float uy, float lz, float uz);
FILE* open_out_file(const char* const inFile, char* mode, char* fileExt);


#endif