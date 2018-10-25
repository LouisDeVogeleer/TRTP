#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "packet_interface.h"
#include "socket.h"
#include <time.h> 
#include <fcntl.h>
#include "queue.h"
struct Queue *buffer;

pkt_t *pkt=NULL;
int lastack;
#define WINDOWSIZE 4
int main(int argc, char *argv[]){
  int wfd=1;
	int isOutFile = 0;         /*  Si = 1, le payload ira vers file. Sinon, le payload vient de STDOUT.*/
	char * file = NULL;
	//char * payload;
	int isAllInt = 0;      /*  Si = 1, on ecoute toutes les interfaces . Sinon, l'hote est précisé dans host.*/
	char * host = NULL;
	int port = 0;
	struct sockaddr_in6 * addr = NULL;
	
	
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
				
		else if(isAllInt == 0 && strcmp(argv[i], "::") == 0){
			isAllInt = 1;
		}
		else if(isAllInt == 0 && host == NULL) {
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
	const char *erro=real_address(host,addr);
	if(erro!=NULL){
	  return EXIT_FAILURE;
	}
	int sfd=create_socket(addr,port,NULL,-1);
	if(sfd<-1){
	  return EXIT_FAILURE;
	}
	

	if(isOutFile == 1){
	  
	  wfd = open(file, O_WRONLY|O_CREAT);
	}
	
	buffer=NewQueue();
	if(buffer==NULL){
	  printf("Erreur New queue");
	}
	socklen_t solen = sizeof(struct sockaddr_in6);
	int eof=0;
	while(!eof){
	  char *buf[528];
	  ssize_t nbre= recvfrom(sfd,buf,528,0,(struct sockaddr *)&addr,&solen);
	  if(nbre==0){
	    eof=1;
	  }
	  else if(nbre<0)
	    {
	      printf("erreur recvfrom");
	    }
	  else{
	    
	    pkt_status_code verifstat=pkt_decode(&buf,sizeof(buf),pkt);
	    if(verifstat!=PKT_OK){
	      printf("Erreur du décodage");
		}
	    else{
	      int seqnum=pkt_get_seqnum(pkt);
	      if(!alreadyQueue(buffer,seqnum)){
		if(isIn(lastack+1,WINDOWSIZE-lastack,pkt_get_window(pkt))){
		
		  if(pkt_get_window(pkt)==lastack+1){
		    lastack+=1;
		    pkt_set_type(pkt, PTYPE_ACK);
		    size_t longu=pkt_get_length(pkt);

		    int err=write(wfd,pkt_get_payload(pkt),longu);
		    if(err==-1){
		      printf("erreur ecriture");
		    }
		    size_t len=524;
		    char send[12];
		    pkt_status_code stat=pkt_encode(pkt,send,&len);
		    if(stat!=PKT_OK){
		      printf("erreur encode");
		    }
		    pkt_del(pkt);
		    err=write(sfd,send,len);
		  }
		  else{
		    int b=enqueue(buffer,pkt);
		    if(b==-1){
		      printf("erreur enqueue");
		    }

		  }
		  }
		  
		  }
	      }
	      
	    }
	  }
	}
	  
	int eof = 0;
	int err;
	char buff [528];
	const char * toWrite;
	while(eof == 0){
		int length = read(sfd, buff, 512);
		
		toWrite = receiving(&buff[0], length);
		if(strcmp(toWrite, "z") == 0){
			eof = 1;
		}
		else{
			if(isOutFile == 1){
				err = fwrite(toWrite, sizeof(char), length, f);
				if(err <= 0) fprintf(stderr, "Erreur : fwrite de %s retourne <=0", file);
			}
			else{
				err = write(1, toWrite, length);
				if(err < 0) fprintf(stderr, "Erreur : write sur stdout retourne <0");
			}
		}
	}
	int isIn(int a, int b, int c){
	  if(c<=b&&c>=a){
	    return 1;
	  }
	  return 0;
	}
