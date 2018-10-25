#ifndef _BUFFER_H
#define _BUFFER_H
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

int isEmpty(Queue q);

int enqueue(Queue * q, pkt_t item);

pkt_t * dequeue(Queue * q);
Queue *NewQueue();
int alreadyQueue(Queue * q, int seqnum);
