#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


//pas oublier de free : file, host
int main(int argc, char *argv[]){
	int isOutFile = 0;         /*  Si = 1, le payload ira vers file. Sinon, le payload vient de STDOUT.*/
	char * file;
	//char * payload;
	int isAllInt = 0;      /*  Si = 1, on ecoute toutes les interfaces . Sinon, l'hote est précisé dans host.*/
	char * host;
	int port;
	
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
	
	printf("-> isOutFile : %d\n", isOutFile);
	if(isOutFile == 1) printf("-> file : %s\n", file);
	printf("-> isAllInt : %d\n", isAllInt);
	if(isAllInt == 0) printf("-> host : %s\n", host);
	printf("-> port : %d\n", port);
}
	