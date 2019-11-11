#ifndef __SINGLE_H_
#define __SINGLE_H_

#include <stdio.h>
#include <stdlib.h>


typedef struct _OctreeNode
{
	char data;
	struct _OctreeNode** children;
} OctreeNode;


#endif