#include "octree.h"
#include "queue.h"


Queue* init_queue()
{
	Queue* Q = malloc(sizeof(*Q));
	Q->data = malloc(INITIAL_CAPACITY * sizeof(*(Q->data)));
	Q->capacity = INITIAL_CAPACITY;
	Q->front = 0;
	Q->rear = -1;
	Q->itemCount = 0;
	return Q;
}


void delete_queue(Queue* Q)
{
	free(Q->data);
	free(Q);
}


static void resize(Queue* Q)
{

}


void* peek(Queue* Q)
{
	return (void*) Q->data[Q->front];
}


bool is_empty(Queue* Q)
{
	return Q->itemCount == 0;
}

int get_size(Queue* Q)
{
	return Q->itemCount;
}


void queue_insert(Queue* Q, void* data)
{
	if (Q->itemCount == Q->capacity)
	{
		resize(Q);
	}

	if (Q->rear == Q->capacity - 1)
	{
		Q->rear = -1;
	}

	Q->data[++(Q->rear)] = data;
	(Q->itemCount)++;
}


void* queue_remove(Queue* Q)
{
	if (is_empty(Q))
	{
		return NULL;
	}

	void* data = Q->data[(Q->front)++];

	if (Q->front == Q->capacity)
	{
		Q->front = 0;
	}

	(Q->itemCount)--;
	return data;
}
