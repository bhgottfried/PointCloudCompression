#ifndef __OCTREE_H_
#define __OCTREE_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <cilk/cilk.h>


extern unsigned int TARGET_DEPTH;
#define NUM_FIELDS 3
#define FIELD_SIZE (sizeof(float))
#define STREAM_PATH_0_IDX 4	// Index in argv that contains the path to the first tree in the stream
#define OUTPUT_PATH_IDX 3


typedef struct _Point
{
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

typedef struct _Link
{
	unsigned char data;
	struct _Link* next;
} Link;

typedef struct _ByteList
{
	unsigned int numBytes;
	Link* head;
	Link* tail;
} ByteList;


// Point functions
PointSet* get_point_set(const char* const fileName);
// Octree functions
OctreeNode* init_node(bool isLeaf);
OctreeNode* create_octree(const PointSet* const ptSet);
bool are_equal(const OctreeNode* const t1, const OctreeNode* const t2);
// Compression functions
ByteList* serialize(const OctreeNode* const root);
OctreeNode* decompress(FILE* const inFilePtr, float* fieldMins, float* fieldMaxs, int* numNodes);
// Diff functions
ByteList* calc_diff(const OctreeNode* curr, const OctreeNode* prev);
OctreeNode* reconstruct_from_diff(const OctreeNode* const prevTree, const ByteList* const diff);
// Merge functions
ByteList* merge_diff(const ByteList* const Dij, const ByteList* const Djk);
OctreeNode** prefix_merge(const OctreeNode* const T0, ByteList** diffs, unsigned int numDiffs);
// Public FileIO functions
void write_pcd_header(FILE* const fp, unsigned int numPoints);
FILE* write_stream_header(const char* const path, unsigned int numClouds);
void write_byte_list(const ByteList* const bl, FILE* const fp);
void write_octree_points(FILE* const fp, const OctreeNode* const root, float lx, float ux, float ly, float uy, float lz, float uz);
FILE* open_out_file(const char* const inFile, char* mode, char* fileExt);
// Memory deallocation and deep copy functions
void delete_point_set(PointSet* ptSet);
void delete_octree(OctreeNode* root);
void delete_byte_list(ByteList* data);
ByteList* copy_byte_list(const ByteList* const bl);
OctreeNode* copy_octree(const OctreeNode* const tree);
// Test functions
void init_test(unsigned int _numDiffs, OctreeNode* T0);
void test(OctreeNode* Ti, ByteList* Di, unsigned int i);
void clean_up_test(int numCompleted);


#endif