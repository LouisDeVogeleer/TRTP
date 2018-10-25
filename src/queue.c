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

int enqueue(Queue * q, pkt_t * item){
	NODE * node = (NODE *) malloc(sizeof(NODE));
	node->item = item;
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

pkt_t * dequeue(Queue * q){
	pkt_t * item = q->tail->item;
	NODE * toFree = q->tail;
	q->tail = q->tail->prev;
	free(toFree);
	return item;	
}

int alreadyQueue(Queue * q, int seqnum){
  if(q->size==0){
    return 0;
  }
  NODE * runner = q->head;
  int i;
  for(i=1; i<=q->size; i++){
    if(pkt_get_numseq(runner->item) == seqnum){
      return 1;
    }
  }
  return 0;  
}
