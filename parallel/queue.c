#include "queue.h"


Queue* init_queue()
{
	Queue* Q = malloc(sizeof(*Q));
	Q->data = malloc(INITIAL_CAPACITY * sizeof(*(Q->data)));
	for (int i = 0; i < INITIAL_CAPACITY; i++)
	{
		Q->data[i] = NULL;
	}

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
	int newDataIdx = 0;
	int newCapacity = 2 * Q->capacity;
	void** newData = malloc(newCapacity * sizeof(*newData));

	// Copy data after front, but before the wrap
	for (int frontIdx = Q->front; frontIdx < Q->capacity; frontIdx++)
	{
		newData[newDataIdx++] = Q->data[frontIdx];
	}

	// Copy wrapped around data
	for (int rearIdx = 0; rearIdx < Q->front; rearIdx++)
	{
		newData[newDataIdx++] = Q->data[rearIdx];
	}

	// Intialize empty values to null
	while (newDataIdx < newCapacity)
	{
		newData[newDataIdx++] = NULL;
	}

	free(Q->data);
	Q->data = newData;
	Q->front = 0;
	Q->rear = Q->capacity - 1;
	Q->capacity *= 2;
}


void* peek(Queue* Q)
{
	if (is_empty(Q))
	{
		return NULL;
	}
	else
	{
		return (void*) Q->data[Q->front];
	}
}


bool is_empty(Queue* Q)
{
	return Q->itemCount == 0;
}


int get_count(Queue* Q)
{
	return Q->itemCount;
}


void enqueue(Queue* Q, const void* const data)
{
	if (Q->itemCount == Q->capacity)
	{
		resize(Q);
	}

	Q->rear = (Q->rear + 1) % Q->capacity;
	Q->data[Q->rear] = data;
	(Q->itemCount)++;
}


void* dequeue(Queue* Q)
{
	if (is_empty(Q))
	{
		return NULL;
	}

	void* data = (void*) Q->data[Q->front];
	Q->front = (Q->front + 1) % Q->capacity;
	(Q->itemCount)--;
	return data;
}
