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
#define WINDOWSIZE 32



pkt_t *pkt=NULL;
int wfd=1;
int sfd=0;
int err=0;
int expSeqnum = 0;

void sendAck(pkt_t* pkt3){
 pkt_set_seqnum(pkt3,expSeqnum);
 pkt_set_type(pkt3, PTYPE_ACK);
 size_t len=528;
 char send[528];
 pkt_status_code stat=pkt_encode(pkt3,send,&len);
 if(stat!=PKT_OK){
   fprintf(stderr, "erreur encode\n");
 }
 pkt_del(pkt3);
 err=write(sfd,send,len);
 if(err<0){
   fprintf(stderr, "erreur write\n");
 }
}
void printPkt(pkt_t* pkt4){
  size_t longu=pkt_get_length(pkt4);

  err=write(wfd,pkt_get_payload(pkt4),longu);//on l'écrit directement dans le fichier
  if(err==-1){
    printf("erreur ecriture");
  }
  pkt_del(pkt4);
}

int main(int argc, char *argv[]){

	int isOutFile = 0;         /*  Si = 1, le payload ira vers file. Sinon, le payload vient de STDOUT.*/
	char * file = NULL;
	//char * payload;
	char * host = NULL;
	int port = 0;
	struct sockaddr_in6 addr;
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
	err = real_address(host,&addr);
	if(err != 0){
		fprintf(stderr, "Error real_adress: %s.\n", gai_strerror(err));
	  return -1;
	}
	sfd=create_socket(&addr,port,NULL,-1);
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

	pkt_t ** buffer=(pkt_t**)malloc(sizeof(pkt_t*)*32);
	for(i=0;i<32;i++){
	  buffer[i]=(pkt_t*)malloc(sizeof(pkt_t));
	  buffer[i]=NULL;
	}
	if(buffer==NULL){
	  fprintf(stderr, "failed to create buffer\n");
	}
	socklen_t solen = sizeof(struct sockaddr_in6);
	int eof1 = 0;


	while(!eof1){//début de la boucle
	  char buf[528];
	  if(buffer[expSeqnum%WINDOWSIZE]!=NULL){//Si l'element qu'on veut est dans la queue
	    expSeqnum+=1;
	    lastack+=1;
	    if(expSeqnum==255){
	      expSeqnum=0;
	    }
	    if(lastack == (WINDOWSIZE-1)){
	      lastack=0;
	    }
	    printPkt(buffer[expSeqnum%WINDOWSIZE]);
	    buffer[expSeqnum%WINDOWSIZE]=NULL;
	  }// fin du si l'element voulue est dans la queue
	  else{
		  memset(&buf, 0, 528);
	    ssize_t nbre= recvfrom(sfd,buf,528,0,(struct sockaddr *)&addr,&solen);
	    fprintf(stderr,"nbre recu :%zd\n",nbre);
	    if(nbre<0) {//Si une erreur
	      fprintf(stderr, "erreur recvfrom\n");
	    }
	    else{//Si on a lu qqch
			pkt=(pkt_t*)malloc(sizeof(pkt_t));
			if(pkt==NULL){
				fprintf(stderr,"erreur malloc");
			}
	      pkt_status_code verifstat=pkt_decode((const char*) buf,nbre,pkt);
	      if(verifstat!=PKT_OK){
			fprintf(stderr, " statut length: %d\n",E_LENGTH);
			fprintf(stderr, " statut CRC: %d\n",E_CRC);
			fprintf(stderr, " statut: %d\n",verifstat);
		    fprintf(stderr, "erreur du décodage1\n");
	      }
	      else{//si on a réussi a le décoder
		//TODO verifier quand il faut un pkt_del(pkt)
		int seqnum=pkt_get_seqnum(pkt);
		if(buffer[expSeqnum%WINDOWSIZE]==NULL){//si c'est pas un element deja dans la queue
		  if(seqnum>=expSeqnum){//Si le seqnum est plus petit que celui attendu on l'ignore
		    if(seqnum%WINDOWSIZE<=WINDOWSIZE-1 && seqnum>=0){//si il est dans l'intervalle de la fenetre recherche
		      if(pkt_get_seqnum(pkt)==expSeqnum){//Si c'est celui attendu

			if(pkt_get_length(pkt)==0 && pkt_get_seqnum(pkt)==expSeqnum){ //Si c'est la fin du fichier
			  eof1=1;
			  pkt_set_type(pkt,1);
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
					buffer[pkt_get_seqnum(pkt)%WINDOWSIZE]=pkt; //On le rajoute dans la queue
					sendAck(pkt);
		      }//fin du else si c'est pas celui attendu
		    }//fin du si il est dans la fenetre attendu
		  }//fin du si le seqnum est plus petit on l'ignore
		}//fin du si ce n'est pas un element de la queue

	      }//fin de si on a reussi a le decoder
	    }// fin du si on a lu qqch
	  }//Si il n'était pas dans la queue

	}//fin de la boucle while
	pkt_del(pkt);
	for(i=0;i<32;i++){
	  free(buffer[i]);
	}
	free(buffer);
}
