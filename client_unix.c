#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <string.h>
#include <limits.h>  //-D_GNU_SOURCE necessario in compilazione per aver IOV_MAX

#include <unistd.h>		//per creare i thread necessari a gestire le varie richieste
#include <sys/socket.h>  //"man sys_socket.h" libreria per gestire i socket
#include <sys/uio.h>  //necessario per writev e readv, permette l'invio di più blocchi di dati

#include <netinet/in.h>  //libreria per ottenere l'indirizzo ip della macchina da usare poi in socket.h
#include <arpa/inet.h> //per convertire da host-bit-order a network-bit-order

#include <poll.h>  //libreria per il polling del file descriptor (attendiamo che ci sia una richiesta i/o)

#define BUFFER 500

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

void print_addr_in(sockaddr_in *addr){
	printf("FAMILY: %d\n", addr->sin_family);
	printf("ADDR: %s PORT: %d\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
}

int file_descriptor(){

	int sockfd = socket(AF_INET,SOCK_STREAM,0); //con ipv4 e sock_stream il protocollo default è TCP

	if(sockfd == -1){
		if(errno == EACCES) printf("Permessi non sufficienti a creare il socket\n");
		else if(errno == EPROTONOSUPPORT) printf("Protocollo non valido per la famiglia specificata");
		return -1;
	}

	return sockfd;
}

int main(int argc, char* argv[]){

    if(argc < 3){
        printf("Usare: '%s [host] [port]'\n", argv[0]);
        return -1;
    }

    sockaddr_in addr_in;
	int sockfd = file_descriptor();

    addr_in.sin_family = AF_INET;
    
    int port = atoi(argv[2]);
    if(!port){
        printf("Porta non valida\n");
        return -1;
    }
    
    addr_in.sin_port = htons(port);
    inet_aton(argv[1],&(addr_in.sin_addr));
    
    if(connect(sockfd, (sockaddr *)&addr_in, sizeof(addr_in)) == -1){
        printf("Errore connessione\n");
        return -1;
    }

    printf("\nConnesso al Server!\n");
	printf("Scrivi un messaggio:\n\n");

	char buffer[1024];
	
	fgets(buffer, 1023, stdin);
	
	write(sockfd, buffer, sizeof(buffer));
	read(sockfd, buffer, 1);
	buffer[1] = '\0';
	
	printf("%s",buffer);
    
}