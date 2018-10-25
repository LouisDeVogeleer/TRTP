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

int enqueue(Queue * q, pkt_t* item){
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

char * dequeue(Queue * q){
  pkt_t * item= (pkt_t *)malloc(sizeof(pkt_t));
	NODE * toFree = q->tail;
	q->tail = q->tail->prev;
	free(toFree);
	return item;	
}
int alreadyQueue(int seqnum,queue *buffer){
  if(size==0){
    return 0;
  }
  NODE * runner=(NODE *)malloc(sizeof(NODE));
  runner=buffer->head;
  for(int i=1;i<=buffer->size;i++){
    
    if(pkt_get_numseq(runner->item)==seqnum){
      return 1;
    }
  }
  return 0;
  
  
  
}
