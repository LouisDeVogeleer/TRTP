#ifndef __QUEUE_H_
#define __QUEUE_H_
#include <stdlib.h> 
#include <stdio.h>
#include "packet_interface.h"

typedef struct node_t{
	pkt_t * item;
	struct node_t * prev;
} NODE;

typedef struct Queue{
	NODE * head;
	NODE * tail;
	int size;
} Queue;

Queue * NewQueue();


int enqueue(Queue * q, pkt_t * item);

pkt_t * seeTail(Queue * q);

pkt_t * dequeue(Queue * q);

#endif
