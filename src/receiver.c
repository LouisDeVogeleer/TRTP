#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "packet_interface.h"
#include <time.h> 

//pas oublier de free : file, host
char * receiving(int sfd, char* buffer, int lenReceived8);

int main(int argc, char *argv[]){
	int isOutFile = 0;         /*  Si = 1, le payload ira vers file. Sinon, le payload vient de STDOUT.*/
	char * file;
	//char * payload;
	int isAllInt = 0;      /*  Si = 1, on ecoute toutes les interfaces . Sinon, l'hote est précisé dans host.*/
	char * host;
	int port;
	struct sockaddr_in6 * addr = NULL;
	
	
	/* Interprétation des arguments */
	for(int i = 1; i < argc; i++){
		
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
			for(int j=0; j<lenhost; j++){
				host[j] = argv[i][j];
				}
		}
		else{
			port = atoi(argv[i]);
		}
	}
	
	/* Creation du socket */
	const char *erro=real_address(host,&addr);
	if(erro!=NULL){
	  return EXIT_FAILURE;
	}
	int sfd=create_socket(&addr,port,NULL,-1);
	if(sfd<-1){
	  return EXIT_FAILURE;
	}
	
	
	FILE *f;
	if(isOutFile == 1){
		f = fopen(file, "w");
	}
	
	int wait = wait_for_client(sfd);
	if(wait == -1){
		fprintf(stderr, "Erreur : aucune donnee recue");
	}
	
	int eof = 0;
	int err;
	char buff[512];
	char * toWrite;
	while(eof == 0){
		int length = read(sfd, buff, 512);
		
		toWrite = receiving(sfd, buff, err);
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
	
}
char * receiving(int sfd, char* buffer, int lenReceived8){
	pkt_t *packet=pkt_new();
	pkt_status_code err=pkt_decode(buffer,lenReceived8,packet);
	if(err!=PKT_OK){
		fprintf(stderr,"error  decode");
	}
	return pkt_get_payload(packet);
}	
