#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "packet_interface.h"
#include "socket.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "queue.h"
#define WINDOWSIZE 4


struct Queue *buffer;
pkt_t *pkt=NULL;

int isIn(int a, int b, int c){
	  if(c<=b&&c>=a){
	    return 1;
	  }
	  return 0;
	}

int alreadyQueue(Queue * q, int seqnum,pkt_t* pkt){
  if(q->size==0){
    return 0;
  }
  NODE * runner = q->head;
  int i;
  for(i=1; i<=q->size; i++){
    if(pkt_get_seqnum(runner->item) == seqnum){
      pkt=runner->item;
      return 1;
    }
  }
  return 0;
}
void sendAck(pkt_t* pkt){
 pkt_set_seqnum(pkt,expSeqnum);
 pkt_set_type(pkt, PTYPE_ACK);
 size_t len=528;
 char send[528];
 pkt_status_code stat=pkt_encode(pkt,send,&len);
 if(stat!=PKT_OK){
   fprintf(stderr, "erreur encode\n");
 }
 pkt_del(pkt);
 err=write(sfd,send,len);
 if(err<0){
   fprintf(stderr, "erreur write\n");
 }
}
void printPkt(pkt_t* pkt){
  size_t longu=pkt_get_length(pkt);
  
  err=write(wfd,pkt_get_payload(pkt),longu);//on l'écrit directement dans le fichier
  if(err==-1){
    printf("erreur ecriture");
  }
}

int main(int argc, char *argv[]){
  int wfd=1;
	int isOutFile = 0;         /*  Si = 1, le payload ira vers file. Sinon, le payload vient de STDOUT.*/
	char * file = NULL;
	//char * payload;
	char * host = NULL;
	int port = 0;
	struct sockaddr_in6 * addr = NULL;
	int lastack=0;


	/* Interprétation des arguments */
	int i;
	for(i = 1; i < argc; i++){

		if(isOutFile == 0 && strcmp(argv[i], "-f") == 0){
			isOutFile = 1;
			int lenfile = strlen(argv[i+1]);
			file = (char *) malloc(lenfile*sizeof(char));
			for(int j=0; j<lenfile; j++){
				file[j] = argv[i+1][j];
			}
			i++;
		}

		else if(host == NULL) {
			int lenhost = strlen(argv[i]);
			host = (char *) malloc(lenhost*sizeof(char));
			int j;
			for(j=0; j<lenhost; j++){
				host[j] = argv[i][j];
				}
		}
		else{
			port = atoi(argv[i]);
		}
	}

	/* Creation du socket */
	int err = real_address(host,addr);
	if(err != 0){
		fprintf(stderr, "Error real_adress: %s.\n", gai_strerror(err));
	  return -1;
	}
	int sfd=create_socket(addr,port,NULL,-1);
	if(sfd<-1){
		fprintf(stderr, "Error create_socket.\n");
	  return -1;
	}


	if(isOutFile == 1){
	  wfd = open(file, O_WRONLY|O_CREAT, S_IWUSR);
		if(wfd < 0){
			perror("open out file");
			return -1;
		}
	}

	buffer=NewQueue();
	if(buffer==NULL){
	  fprintf(stderr, "failed to create buffer\n");
	}
	socklen_t solen = sizeof(struct sockaddr_in6);
	int eof1 = 0;
	int expSeqnum = 0;

	while(!eof1){//début de la boucle
	  char *buf[528];
	  pkt_t* pkt2=(pkt_t *)malloc(sizeof(pkt_t));
	  if(pkt2==NULL){
	    fprintf(stderr,"erreur allocation");
	      }
	  if(alreadyQueue(buffer,expSeqnum,pkt2)){//Si l'element qu'on veut est dans la queue
	    
	    
	    expSeqnum+=1;
	    lastack+=1;
	    if(expSeqnum==255){
	      expSeqnum=0;
	    }
	    if(lastack == (WINDOWSIZE-1)){
	      lastack=0;
	    }
	   
	    printPkt(pkt2);
	    pkt_del(pkt2);
		}// fin du si l'element voulue est dans la queue
	  else{
			ssize_t nbre= recvfrom(sfd,buf,528,0,(struct sockaddr *)&addr,&solen);

	    if(nbre<0) {//Si une erreur
			  fprintf(stderr, "erreur recvfrom\n");
			}
	    else{//Si on a lu qqch
				memset(&buf, 0, 548);
				pkt_status_code verifstat=pkt_decode((const char*) buf,sizeof(buf),pkt);
	      if(verifstat!=PKT_OK){
					fprintf(stderr, "erreur du décodage\n");
	      }
	      else{//si on a réussi a le décoder
					//TODO verifier quand il faut un pkt_del(pkt)
					int seqnum=pkt_get_seqnum(pkt);
					if(!alreadyQueue(buffer,seqnum)){//si c'est pas un element deja dans la queue
						if(seqnum>=expSeqnum){//Si le seqnum est plus petit que celui attendu on l'ignore
							if(isIn(lastack,WINDOWSIZE-lastack,pkt_get_window(pkt))){//si il est dans l'intervalle de la fenetre recherche
								if(pkt_get_seqnum(pkt)==expSeqnum){//Si c'est celui attendu
									
									if(pkt_get_length(pkt)==0 && pkt_get_seqnum(pkt)==expSeqnum){ //Si c'est la fin du fichier
									  eof1=1;
									  sendAck(pkt);
									}//fin de fin du fichier
									else{
										expSeqnum+=1;
										lastack+=1;
										if(expSeqnum==255){
										  expSeqnum=0;
										}
										if(lastack==WINDOWSIZE-1){
										  lastack=0;
										}
										printPkt(pkt);
										sendAck(pkt);
									}
								}//Fin du si c'est celui attendu
								else{//Si c'est pas celui attendu
									int b=enqueue(buffer,pkt);//On le rajoute dans la queue
									if(b==-1){
										fprintf(stderr, "erreur enqueue\n");
									}
									sendAck(pkt);
								}//fin du else si c'est pas celui attendu
							}//fin du si il est dans la fenetre attendu
						}//fin du si le seqnum est plus petit on l'ignore
					}//fin du si ce n'est pas un element de la queue
				}//fin de si on a reussi a le decoder
			}// fin du si on a lu qqch
		}//Si il n'était pas dans la queue
	}//fin de la boucle while
}
