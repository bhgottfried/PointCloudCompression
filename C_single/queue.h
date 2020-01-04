#ifndef __QUEUE_H_
#define __QUEUE_H_

#include <stdlib.h>
#include <stdbool.h>


#define INITIAL_CAPACITY 256


typedef struct _Queue
{
	void** data;
	int capacity;
	int front;
	int rear;
	int itemCount;
} Queue;


Queue* init_queue();
void delete_queue(Queue* Q);

void* peek(Queue* Q);
bool is_empty(Queue* Q);
int get_size(Queue* Q);
void queue_insert(Queue* Q, void* data);
void* queue_remove(Queue* Q);


#endif
