#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>

//#include "packet_implem.c"
#include "socket.h"
#include "queue.h"

#define WINDOWSIZE 32



uint8_t seqNum = 0 ;
char bufWrite[528];
int sfd = -1;


pkt_t * createPacket(char * payload, uint16_t length){
	pkt_t * packet=pkt_new();
	pkt_set_type(packet,PTYPE_DATA);
	pkt_set_window(packet, seqNum % WINDOWSIZE);
	pkt_set_seqnum(packet, seqNum);
	pkt_set_timestamp(packet, clock()/CLOCKS_PER_SEC);
	pkt_set_payload(packet, payload, length);
	pkt_set_length(packet, length);

	seqNum ++;
	if(seqNum == 255){
		seqNum = 0;
	}
	return packet;
}

int sendPacket(int fd, pkt_t * packet, size_t dataLen){
	memset((void*) bufWrite, 0, 528);
	if(pkt_encode(packet, bufWrite, &dataLen) != PKT_OK){
		fprintf(stderr, "failed to encode\n");
		return -1;
	}

	if(write(fd, bufWrite, dataLen)  < 0){
		perror("failed to write sendPacket\n");
		return -1;
	}
	return 0;
}


int main(int argc, char *argv[]){
	int isInFile = 0;         /*  Si = 1, le payload vient de file. Sinon, le payload vient de STDIN.*/
	char * file = NULL;
	char * host = NULL;
	int port = 0;
	struct sockaddr_in6 addr;
	pkt_t * recPacket = NULL;
	uint32_t RTT = 5;
	int err;

  /* Interprétation des arguments. */
	int i;
	for(i = 1; i < argc; i++){
		if(isInFile == 0 && strcmp(argv[i], "-f") == 0){
			isInFile = 1;
			int lenfile = strlen(argv[i+1]);
			file = (char *) malloc((1+lenfile)*sizeof(char));
			int j;
			for(j=0; j<=lenfile; j++){
				file[j] = argv[i+1][j];
			}
			i++;
		}

		else if(host == NULL) {
			int lenhost = strlen(argv[i]);
			host = (char *) malloc((1+lenhost)*sizeof(char));
			int j;
			for(j=0; j<=lenhost; j++){
				host[j] = argv[i][j];
				}
		}
		else{
			port = atoi(argv[i]);
		}
	}

	/* Creation du socket. */
	memset(&addr, 0, sizeof(struct sockaddr_in6));
	err=real_address(host, &addr);
	free(host);
	if(err != 0){
		fprintf(stderr, "Error real_adress: %s.\n", gai_strerror(err));
		return -1;
	}
	sfd = create_socket(NULL, -1, &addr, port);
	if(sfd == -1){
		fprintf(stderr, "Error create_socket.\n");
		return -1;
	}


	/* Lecture du fichier ou de STDIN et creation des pkt. */
	int rfd = 0;
	char payload[512];
	char bufRead[528];
	fd_set rfds;
	int readRet = 1;
	pkt_t * newPacket = NULL;
	int lastAck = 0;
	int recLastAck = 0;
	int sentEndPacket = 0;

	if(isInFile == 1){
		rfd= open(file, O_RDONLY);
		free(file);
	}

	int maxfd = rfd ;
	if(sfd > rfd) maxfd = sfd ;

	Queue * q = NewQueue();
	if(q == NULL){
		fprintf(stderr, "q == NULL");
	}

	while(recLastAck == 0){
		int currentTime = 0;
		struct timeval  tv;
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		FD_ZERO(&rfds);
		FD_SET(sfd, &rfds);
		if(q->size < WINDOWSIZE-1 && sentEndPacket == 0){
			FD_SET(rfd, &rfds);
		}

		err = select(maxfd + 1, &rfds, NULL, NULL, &tv);

		if(err== -1) perror("select");
		if(err > 0){
			/* Acces a la lecture de STDIN ou du fichier d'entree. */
			if(FD_ISSET(rfd, &rfds) && sentEndPacket == 0){
				memset((void*) payload, 0, 512);
				readRet = read(rfd, payload, 512);


				/* Creation d'un packet standard. */
				if(readRet > 1){
					newPacket = createPacket(payload, readRet);
				}

				/* Creation du paquet de deconnexion. */
				else if((readRet == 1 && isInFile == 0) || readRet == 0){
					newPacket = createPacket("", 0);
					sentEndPacket = 1;
				}

				else perror("failed to read input\n");

				if(enqueue(q, newPacket) != 0){
					fprintf(stderr, "failed to enqueue newPacket\n");
					pkt_del(newPacket);
					freeQueue(q);
					return -1;
				}
				fprintf(stderr, "    queue is size %d\n", q->size);
				
				if(sendPacket(sfd, newPacket, readRet +16) != 0){
					pkt_del(newPacket);
					freeQueue(q);
					return -1;
				}
				fprintf(stderr, "--- sent data %d\n", pkt_get_seqnum(newPacket));
			}

			/* Acces a la lecture des donnees du reseau. */
			if(FD_ISSET(sfd, &rfds)){
				recPacket = pkt_new();
				memset((void*) bufRead, 0, 528);
				if((err = read(sfd, bufRead, 528)) > 0){
					if(pkt_decode((const char *) bufRead, err, recPacket) != PKT_OK){
						pkt_del(recPacket);
						fprintf(stderr, "failed to decode data\n");
					}

					/* ACK */
					if(pkt_get_type(recPacket) == 2){
						
						lastAck = pkt_get_seqnum(recPacket);
						fprintf(stderr, "*** received ACK : sq_tail=%d lastAck=%d\n",pkt_get_seqnum(seeTail(q)), lastAck);
						while(q->size != 0 && (pkt_get_seqnum(seeTail(q))%WINDOWSIZE) < lastAck){
							dequeue(q);
						}

						if(pkt_get_timestamp(recPacket) < RTT){
							RTT = pkt_get_timestamp(recPacket);
						}
					}

					/* NACK */
					if(pkt_get_type(recPacket) == 3 && q->size!= 0){
						int sq = pkt_get_seqnum(recPacket);
						fprintf(stderr, "*** received NACK %d\n", sq);
						NODE * runner = q->head;
						for(i=0; i<q->size; i++){
							if(pkt_get_seqnum(runner->item) == sq){
								if(pkt_set_timestamp(runner->item, clock()/CLOCKS_PER_SEC) != PKT_OK){
									fprintf(stderr, "failed to reset timestamp\n");
									freeQueue(q);
									return -1;
								}
								if(sendPacket(sfd, runner->item, pkt_get_length(runner->item) + 16) != 0){
									pkt_del(recPacket);
									freeQueue(q);
									return -1;
								}
							}
							runner = runner->prev;
						}
					}

					if(pkt_get_type(recPacket) == 1 && pkt_get_length(recPacket) == 0 && pkt_get_seqnum(recPacket) == lastAck){
						recLastAck = 1;
					}
				}
				pkt_del(recPacket);
			}

			/* Verification des timer et renvoi des paquets non-acquites. */
			currentTime = clock() / CLOCKS_PER_SEC;
			NODE * runner = q->tail;
			for(i=1; i<=q->size; i++){
				if((currentTime - pkt_get_timestamp(runner->item)) > (RTT + 2) ){
					//fprintf(stderr, "clock check\n");
					//fprintf(stderr, "   currentTime: %d\n", currentTime);
					//fprintf(stderr, "   get_timestamp: %d\n", pkt_get_timestamp(runner->item));
					//fprintf(stderr, "   RTT: %d\n", RTT);
					if(pkt_set_timestamp(runner->item, clock()/CLOCKS_PER_SEC) != PKT_OK){
						fprintf(stderr, "failed to reset timestamp\n");
						freeQueue(q);
						return -1;
					}
					if(sendPacket(sfd, runner->item, pkt_get_length(runner->item) + 16) != 0){
						freeQueue(q);
						return -1;
					}
				}
				runner = runner->prev;
			}
		}
	}

	freeQueue(q);
	return 0;
}
