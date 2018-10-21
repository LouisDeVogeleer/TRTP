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
	char payload [512];
	struct sockaddr_in6 addr;
	
    /* Interprétation des arguments */	
	for(int i = 1; i < argc; i++){
		if(isInFile == 0 && strcmp(argv[i], "-f") == 0){
			isInFile = 1;
			int lenfile = strlen(argv[i+1]);
			file = (char *) malloc(lenfile*sizeof(char));
			for(int j=0; j<lenfile; j++){
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
			for(int j=0; j<lenhost; j++){
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
	
	FILE *f;
	if(isInFile == 1){
		f = fopen(file, "r");
	}
	
	while(eof == 0){
		
		if(isInFile == 1){
			err = fread(payload, sizeof(char), 512, f);
			if(err <= 0) fprintf(stderr, "Erreur : fread de %s retourne <=0", file);
			eof = feof(f);
		}
		else{
			err = read(0, payload, 512);
			if(err < 0) fprintf(stderr, "Erreur : read de stdin retourne <0");
			if(err == 0) eof = 1;
		}	
		sending(sfd, payload, err);
		
	}
	
	sending(sfd, "z", 1);
	sending(sfd, "z", 1);
	sending(sfd, "z", 1);
	
	free(host);
	free(file);
	
	return 0;
}


void sending(int sfd, char * payload, uint16_t length){
	size_t maxLen = 8 + length;
	char buff[maxLen];
	pkt_t * packet=pkt_new();
	pkt_set_type(packet,PTYPE_DATA);
	pkt_set_window(packet,4);
	pkt_set_seqnum(packet, numSeq);
	pkt_set_timestamp(packet,clock()/CLOCKS_PER_SEC);
	pkt_set_payload(packet, payload, length);
	pkt_set_length(packet, length);
	if(pkt_encode(packet, buff, &maxLen) != PKT_OK){
		fprintf(stderr,"failed to encode");
	}  
	if(write(sfd, buff, maxLen)<0){
		fprintf(stderr,"failed to write on the socket");
	}
	pkt_del(packet);
	numSeq++;
}
			