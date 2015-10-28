#ifndef _queue_h_
#define _queue_h_

#include <pthread.h>

#ifndef NULL
#define NULL (void*)0
#endif // !NULL

struct QueueData
{
	struct QueueData * next, * previous;
	char * data;
};
struct Queue
{
	pthread_mutex_t token;
	int data_len;
	unsigned int queue_length;
	struct QueueData * head, * tail;
};

typedef struct Queue Queue_t;

extern int QueueCreate(Queue_t * queue, int data_len);
extern int QueueEnQueue(Queue_t * queue, void * data);
extern int QueueDeQueue(Queue_t * queue, void * data);
extern int QueueDestroy(Queue_t * queue);
#endif // !_queue_h_
