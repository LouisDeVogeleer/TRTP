#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h> 

#include "packet_interface.h"
#include "socket.h"



uint8_t numSeq = 0 ;
void sending(int sfd, char* payload, uint16_t length);

int main(int argc, char *argv[]){
	int isInFile = 0;         /*  Si = 1, le payload vient de file. Sinon, le payload vient de STDIN.*/
	char * file = NULL;
	//char * payload;
	int isLocal = 0;      /*  Si = 1, l'hote est local. Sinon, il est précisé dans host.*/
	char * host = NULL;
	int port = 0;
	struct sockaddr_in6 addr;
	
    /* Interprétation des arguments */
	int i;
	for(i = 1; i < argc; i++){
		if(isInFile == 0 && strcmp(argv[i], "-f") == 0){
			isInFile = 1;
			int lenfile = strlen(argv[i+1]);
			file = (char *) malloc(lenfile*sizeof(char));
			int j;
			for(j=0; j<lenfile; j++){
				file[j] = argv[i+1][j];
			}
			i++;
		}
				
		else if(isLocal == 0 && strcmp(argv[i], "::1") == 0){
			isLocal = 1;
		}
		else if(isLocal == 0 && host == NULL) {
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
	const char *erro=real_address(host, &addr);
	if(erro != NULL){
	  return EXIT_FAILURE;
	}
	int sfd=create_socket(NULL, -1, &addr, port);
	if(sfd == -1){
	  return EXIT_FAILURE;
	}
	
	
	/* Lecture du fichier ou de STDIN et creation des pkt*/
	int err;
	int eof = 0;
	int rfd = 0;
	char payload [512];
	char bufWrite[528];
	char bufRead[528];
	fd-set rfds;
	struct timeval  tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	int readRet = 1;
	isBufferFull =0;
	pkt_t * newPacket = NULL;

	
	if(isInFile == 1){
		rfd= open(file, O_RDONLY);
	}
	
	int maxfd = rfd ;
	if(sfd > rfd) maxfr = sfd ;
	
	//TODO creer le buffer
	
	while(readRet != EOF){
		FD_ZERO(&rfds);
		FD_SET(sfd &rfds);
		if( isBufferFull == 0){
			FD_SET(rfd, &rfds);
		}
		
		err = select(maxfd + 1, &rfds, NULL, NULL);
		
		if(ret == -1) perror("select");
		if(ret > 0){
			/* Acces a la lecture de STDIN ou du fichier d'entree */
			if(FD_ISSET(rfd, &rfds){
				memset((void*) payload, 0, 512);
				if(  (readRet =  read(rfd, payload, 512))  > 0){
					int dataLen = readRet + 16;
					newPacket = createPacket(payload, readRet);
					
					//TODO ajouter dans le buffer
					
					memset((void*) bufWrite, 0, 528);
					if(pkt_encode(packet, bufWrite, &datLen) != PKT_OK){
						fprintf(stderr, "failed to encode");
					}  
					
					if(write(sfd, bufWrite, dataLen)  < 0){
						perror("failed to write on sfd");
					}
					
					pkt_del(newPacket);
				}
				else perror("failed to read input")
			}
			
			/* Acces a la lecture des donnees du reseau */
			if(FD_ISSET(sfd, &rfds){
				memset((void*) bufRead, 0, 528);
				if(read(sfd, bufRead, 528){
					//TODO cas ACK et maj du buffer
					//TODO cas NACK, quid ?
				}
			}
			
			//TODO checker timer dans la queue et renvoyer si besion
		}
	}
	
	free(host);
	free(file);

	return 0;
}

/*
err = read(0, payload, 512);
if(err < 0) fprintf(stderr, "Erreur : read de stdin retourne <0");
if(err == 0) eof = 1;
sending(sfd, payload, err);
*/


pkt_t * createPacket(char * payload, uint16_t length){
	size_t maxLen = 16 + length;
	pkt_t * newPacket = (pkt_t *) malloc(sizeof(pkt_t));
	newPacket=pkt_new();
	pkt_set_type(packet,PTYPE_DATA);
	pkt_set_window(packet, 4);
	pkt_set_seqnum(packet, numSeq);
	pkt_set_timestamp(packet, timestamp);
	pkt_set_payload(packet, payload, length);
	pkt_set_length(packet, length);
	
	if(numSeq == 255){
		numSeq = 0;
	}
	numSeq++;
	
	return newPacket;	
}
			