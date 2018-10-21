#include <queue.h>
#include <stdlib.h> 
#include <stdio.h>


Queue * NewQueue(){
	Queue * q = (Queue *) malloc(sizeof(Queue));
	q->head = NULL;
	q->tail = NULL;
	q->size = 0;
	return q;
}

int isEmpty(Queue q){
	return q->size == 0;
}

int enqueue(Queue * q, char * payload){
	NODE * node = (NODE *) malloc(sizeof(NODE));
	node->payload = payload;
	node->prev = NULL;
	
	if(q->size == 0){
		q->head = node;
		q->prev = node;
	}
	else{
		q->head->prev = node;
		q->head = node;
	}
	q->size ++;
}

char * dequeue(Queue * q){
	char * payload = q->tail->payload;
	NODE * toFree = q->tail;
	q->tail = q->tail->prev;
	free(toFree);
	return payload;	
}
