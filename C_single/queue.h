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
int get_count(Queue* Q);
void enqueue(Queue* Q, const void* const data);
void* dequeue(Queue* Q);


#endif
