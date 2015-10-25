/*
 * queue.h
 *
 *  Created on: Sep 26, 2015
 *      Author: kieutq
 */

#ifndef SRC_QUEUE_H_
#define SRC_QUEUE_H_

typedef int Queue_t;

int Queue_Init(Queue_t * queue, int queue_size, int data_size);
int Queue_Send(Queue_t * queue, void * data, int data_size);
int Queue_Receive(Queue_t * queue, void * data, int data_size);
int Queue_Length(Queue_t * queue);

#endif /* SRC_QUEUE_H_ */
