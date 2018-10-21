#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>


//pas oublier de free : file, host
int main(int argc, char *argv[]){
	int isInFile = 0;         /*  Si = 1, le payload vient de file. Sinon, le payload vient de STDIN.*/
	char * file;
	//char * payload;
	int isLocal = 0;      /*  Si = 1, l'hote est local. Sinon, il est précisé dans host.*/
	char * host;
	int port;
	char * payload;
	

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
	
	
	/* Lecture du fichier ou de STDIN pour creer le payload */
	payload = (char *) malloc(512*sizeof(char));
	int err;
	
	if(isInFile == 1){
		FILE * f = fopen(file, "r");
		err = fread(payload, sizeof(char), 512, f);
		if(err <= 0){
			fprintf(stderr, "Erreur : fread de %s retourne <=0", file);
		}
	}
	else{
		err = read(0, payload, 512);
		if( err<0){
			fprintf(stderr, "Erreur : read de stdin retourne <0");
		}
	}
	return port;
}
	