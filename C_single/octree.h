#ifndef __OCTREE_H_
#define __OCTREE_H_


#define _CRT_SECURE_NO_DEPRECATE	// So fopen doesn't throw deprecation warning
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#define NUM_FIELDS 3
#define FIELD_SIZE (sizeof(float))


typedef struct _Point
{
	float coords[NUM_FIELDS];
} Point;

typedef struct _OctreeNode
{
	Point* data;
	struct _OctreeNode* children[8];
} OctreeNode;


// Point functions
unsigned int parse_header(FILE* const fp);
Point** read_points(FILE* const fp, unsigned int numPoints);
void get_min_max(Point** points, unsigned int numPoints, float* fieldMins, float* fieldMaxs);
// Octree functions
OctreeNode* create_octree(Point** points, unsigned int numPoints, float* fieldMins, float* fieldMaxs);
void delete_octree(OctreeNode* root);


#endif