#ifndef __OCTREE_H_
#define __OCTREE_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define TARGET_DEPTH 3	// Can be added into the input file header, but will violate the .pcd standard
#define NUM_FIELDS 3
#define FIELD_SIZE (sizeof(float))


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
	Point* data;
	struct _OctreeNode* children[8];
} OctreeNode;


// Point functions
PointSet* get_point_set(const char* const fileName);
void delete_point_set(PointSet* ptSet);
void write_header(FILE* const fp, unsigned int numPoints);
// Octree functions
OctreeNode* init_node(Point* data);
OctreeNode* create_octree(const PointSet* const ptSet);
void write_octree_points(FILE* const fp, const OctreeNode* const root, int depth, float lx, float ux, float ly, float uy, float lz, float uz);
void delete_octree(OctreeNode* root);
// Compression functions
FILE* open_out_file(const char* const inFile, char* mode, char* fileExt);
void compress(const OctreeNode* const root, FILE* const fp);
void decompress(FILE* const inFilePtr, FILE* const outFilePtr);


#endif