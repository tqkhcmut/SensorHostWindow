#include "queue.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


int QueueCreate(Queue_t * queue, int data_len)
{
	if (queue->head != NULL)
		QueueDestroy(queue);
	pthread_mutex_init(&queue->token, NULL);
	queue->data_len = data_len;
	queue->head = NULL;
	queue->tail = NULL;
	queue->data_len = 0;
	queue->queue_length = 0;
	return 0;
}
int QueueEnQueue(Queue_t * queue, void * data)
{
	struct QueueData * newdata;
	if (queue->data_len == 0)
		// something wrong with data length, may be queue not created
		return -1;
	newdata->data = malloc(queue->data_len);
	memcpy(newdata->data, data, queue->data_len);
	newdata->next = queue->head;
	newdata->previous = NULL;
	queue->head = newdata;
	if (queue->tail == NULL)
	{
		queue->tail->previous = newdata;
	}
	queue->queue_length++;
	return 0;
}
int QueueDeQueue(Queue_t * queue, void * data)
{
	struct QueueData * tmpdata;
	if (queue->queue_length == 0)
		return -1; // queue empty
	tmpdata = queue->tail;
	queue->tail = queue->tail->previous;
	queue->tail->next = NULL;
	if (data != NULL) // set data equal NULL to ignore dequeue data
		memcpy(data, tmpdata->data, queue->data_len);
	free(tmpdata->data);
	queue->queue_length--;
	return 0;
}
int QueueDestroy(Queue_t * queue)
{
	struct QueueData * tmpdata;
	if (queue->queue_length == 0)
		return -1; // queue empty
	tmpdata = queue->head;
	while (tmpdata != NULL)
	{
		free(tmpdata->data);
		tmpdata = tmpdata->next;
	}
	queue->data_len = 0;
	queue->head = NULL;
	queue->tail = NULL;
	queue->queue_length = 0;
	return 0;
}

