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

typedef struct _OctreeNode
{
	Point* data;
	struct _OctreeNode* children[8];
} OctreeNode;


// Point functions
unsigned int parse_header(FILE* const fp);
void write_header(FILE* const fp, unsigned int numPoints);
Point** read_points(FILE* const fp, unsigned int numPoints);
void get_min_max(Point** points, unsigned int numPoints, float* fieldMins, float* fieldMaxs);
// Octree functions
OctreeNode* init_node(Point* data);
OctreeNode* create_octree(Point** points, unsigned int numPoints, float* fieldMins, float* fieldMaxs);
void write_octree_points(FILE* const fp, const OctreeNode* const root, int depth, float lx, float ux, float ly, float uy, float lz, float uz);
void delete_octree(OctreeNode* root);
// Compression functions
FILE* open_out_file(const char* const inFile, char* mode, char* fileExt);
void compress(const OctreeNode* const root, FILE* const fp);
void decompress(FILE* const inFilePtr, FILE* const outFilePtr);


#endif