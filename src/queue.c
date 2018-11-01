#include "queue.h"
#include <stdlib.h>
#include <stdio.h>


Queue * NewQueue(){
	Queue * q = (Queue *) malloc(sizeof(Queue));
	q->head = NULL;
	q->tail = NULL;
	q->size = 0;
	return q;
}

void freeQueue(Queue * q){
	if(q == NULL) return;
	NODE * toFree = NULL;
	while(q->tail != NULL){
		toFree = q->tail;
		q->tail = q->tail->prev;
		free(toFree);
	}
	free(q);
}

int enqueue(Queue * q, pkt_t * item){
	NODE * node = (NODE *) malloc(sizeof(NODE));
	if(node == NULL) return -1;
	node->item = item;
	node->prev = NULL;

	if(q->size == 0){
		q->head = node;
		q->tail = node;
	}
	else{
		q->head->prev = node;
		q->head = node;
	}
	q->size ++;
	return 0;
}

pkt_t * dequeue(Queue * q){
	pkt_t * item = q->tail->item;
	NODE * toFree = q->tail;
	q->tail = q->tail->prev;
	pkt_del(toFree->item);
	free(toFree);
	q->size --;
	return item;
}

pkt_t * seeTail(Queue * q){
	return q->tail->item;
}
