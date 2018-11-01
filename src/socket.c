#include <stdlib.h>
#include <stdio.h> //printf
#include <netinet/in.h> //struct addrin_6
#include <string.h> //memset
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include "socket.h"

int real_address(const char *address, struct sockaddr_in6 *rval){
    struct addrinfo * result = NULL;
    struct addrinfo hints;
    hints.ai_flags = 0;
    hints.ai_family =  AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int a = getaddrinfo(address,NULL, &hints, &result);
    if(a!=0){
      printf("getaddrinfo return : %d\n", a);
      freeaddrinfo(result);
      return a;
    }
    struct sockaddr_in6 * result1 = (struct sockaddr_in6 *)(result->ai_addr);
    *rval = *result1;
    freeaddrinfo(result);
    return 0;
}


int create_socket(struct sockaddr_in6 *source_addr,
                 int src_port,
                 struct sockaddr_in6 *dest_addr,
                 int dst_port)
{

    struct protoent * proto = getprotobyname("udp");
    int a=socket(AF_INET6,SOCK_DGRAM,proto->p_proto);
    if(a==-1){
        fprintf(stderr, "ERROR : creation de socket\n");
        return a;
    }

    if(source_addr!=NULL && src_port>0){
        source_addr->sin6_port = htons(src_port);
        int b=bind(a, (struct sockaddr *) source_addr,(socklen_t) sizeof(struct sockaddr_in6));
        if(b!=0){
            perror("bind");
            return b;
        }
    }

    if(dest_addr!=NULL && dst_port>0){
        dest_addr->sin6_port = htons(dst_port);
		//fprintf(stderr,"sfd : %d\n", a);
		//fprintf(stderr,"port : %d\n", dst_port);
		//fprintf(stderr,"port of struct : %d\n", dest_addr->sin6_port);
		//fprintf(stderr,"sfd : %d\n", a);
        int c=connect(a, (const struct sockaddr *) dest_addr,(socklen_t) sizeof(struct sockaddr_in6));
        if(c!=0){
            perror("connect");
            return c;
        }
    }

    return a;
}


int wait_for_client(int sfd){
    char buffer[1024];
    socklen_t solen = sizeof(struct sockaddr_in6);
    struct sockaddr_in6 * a6 = (struct sockaddr_in6 *) calloc(1, sizeof(struct sockaddr_in6));

    //utilisés pour recevoir des messages depuis une socket et renvoie le nombre d'octets reçus
    //MSG_PEEK Cet attribut permet de lire les données en attente dans la file sans les enlever de cette file

    int a = recvfrom(sfd, buffer, 1024*sizeof(char), MSG_PEEK, (struct sockaddr *) &a6, &solen);
    if(a==-1){
        perror("recvfrom");
        return -1;
    }
    // connecte la socket référencée par le descripteur de fichier sockfd à l'adresse indiquée par serv_addr
    a = connect(sfd, (struct sockaddr *) &a6, solen);
    if(a==-1){
      perror("connect");
      return -1;
    }
    return 0;
  }


void read_write_loop(int sfd){
    fd_set rfds;
    struct timeval tv;
    int ret;
    int readSTDIN = 1;

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    char * bufRead = (char *) malloc(1024*sizeof(char));
    char * bufWrite = (char *) malloc(1024*sizeof(char));
    if(bufRead == NULL || bufWrite == NULL){
        fprintf(stderr, "ERROR : malloc bufRead or bufWrite\n");
        exit(0);
    }

    while(readSTDIN != EOF){
        memset((void *) bufRead, 0, 1024);
        memset((void *) bufWrite, 0, 1024);
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        FD_SET(sfd, &rfds);
        ret = select(sfd+1, &rfds, NULL, NULL, &tv);

        if(ret == -1) perror("select");
        if(ret > 0){
            if(FD_ISSET(0, &rfds)){
                if( (readSTDIN = read(0, bufWrite, 1024*sizeof(char))) > 0){
                    fprintf(stderr, "read on stdin : %s\n", bufWrite);
                    if(write(sfd, (void *) bufWrite, readSTDIN) < 0){
                        perror("ERROR write std");
                    }
                    fprintf(stderr, "sent\n");
                }
                else perror("ERROR read stdin");
            }
            if(FD_ISSET(sfd, &rfds)){
                fprintf(stderr, "suis dans la lecture de sfd\n");
                if( (ret = read(sfd, (void *) bufRead, 1024*sizeof(char))) > 0){
                    fprintf(stderr, "read on sfd : %s\n", bufRead);
                    if(write(1, (void *) bufRead, ret) < 0){
                        perror("ERROR write stdout");
                    }
                    fprintf(stderr, "sent\n");
                }
                else perror("ERROR read sfd");
            }
        }
    }
    free(bufRead);
    free(bufWrite);
}
