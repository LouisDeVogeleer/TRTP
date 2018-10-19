#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


//pas oublier de free : file, host
int main(int argc, char *argv[]){
	int isInFile = 0;         /*  Si = 1, le payload vient de file. Sinon, le payload vient de STDIN.*/
	char * file;
	//char * payload;
	int isLocal = 0;      /*  Si = 1, l'hote est local. Sinon, il est précisé dans host.*/
	char * host;
	int port;
		
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
	
	printf("-> isInFile : %d\n", isInFile);
	if(isInFile == 1) printf("-> file : %s\n", file);
	printf("-> isLocal : %d\n", isLocal);
	if(isLocal == 0) printf("-> host : %s\n", host);
	printf("-> port : %d\n", port);
}
	