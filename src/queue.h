#ifndef _BUFFER_H
#define _BUFFER_H
#include <stdlib.h> 
#include <stdio.h>

typedef struct node_t{
	char * payload;
	struct node_t * prev;
} NODE;

typedef struct Queue{
	NODE * head;
	NODE * tail;
	int size;
} Queue;

int isEmpty(Queue q);

int enqueue(Queue * q, char * payload);

char * dequeue(Queue * q);