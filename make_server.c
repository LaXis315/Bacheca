#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h> //-D_GNU_SOURCE necessario in compilazione per aver IOV_MAX
#include <signal.h>

#include <string.h>

#include <unistd.h>		//per creare i thread necessari a gestire le varie richieste
//#include <pthread.h>   //per creare i thread e utilizzare mutex
#include <sys/socket.h>  //"man sys_socket.h" libreria per gestire i socket
#include <sys/uio.h>
#include <netinet/in.h>  //libreria per ottenere l'indirizzo ip della macchina da usare poi in socket.h
#include <arpa/inet.h> //per convertire da host-bit-order a network-bit-order
#include <netdb.h> //da utilizzare per ottenere facilmente interfacce del computer e creare sockaddr
#include <poll.h>  //libreria per il polling del file descriptor (attendiamo che ci sia una richiesta i/o)

#include "manager_requests.c"

#define MAX_QUEUE 2 //numero massimo di richieste di connessioni permesse in un dato momento

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

int ListenSocket; //Main ListenSocket for Listening

bool manager_requests(int sockfd_conn);

void interrupt_handler(int sign){
	if(sign != SIGINT) return;

	printf("\nInterruzione del server e disconnessione del socket\n");
	close(ListenSocket);
	exit(0);
}

int make_socket(char* port){

	struct addrinfo hints;
	struct addrinfo *result;
	int sockfd;

	memset(&hints, 0, sizeof(hints)); //resettiamo hints

	hints.ai_family = AF_INET;    /* Allow IPv4*/
    hints.ai_socktype = SOCK_STREAM; /* stream socket */
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
   	hints.ai_next = NULL;

   	int value = getaddrinfo(NULL, port, &hints, &result);  //qua stiamo indicando una porta specifica e indirizzo qualsiasi

   	if (value != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(value));
        exit(EXIT_FAILURE);
    }

	/*ORA ESEGUIAMO IL BINDING PER LA PRIMA STRUTTURA TROVATA*/

    sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sockfd == -1){
    	freeaddrinfo(result);
        exit(-1);
    }

    errno = 0;
    if(bind(sockfd, result->ai_addr, result->ai_addrlen) == -1){
		if(errno == EACCES) printf("L'indirizzo con qui si sta eseguendo il binding e' protetto.\n");
		else if(errno == EADDRINUSE) printf("Indirizzo bindato gia in uso\n");
		else if(errno == EINVAL) printf("addrlen e'sbagliato oppure addr non e'un indirizzo valido\n");
		else if(errno == EADDRNOTAVAIL) printf("Interfaccia non esistente\n");
		else if(errno == ENAMETOOLONG) printf("address troppo lungo\n");
		fprintf(stderr, "Error binding\n");
		close(sockfd);
		freeaddrinfo(result);
		exit(-1);
	}

	if (signal(SIGINT, interrupt_handler) == SIG_ERR) printf("\ncan't catch SIGINT\n");
  		  
    freeaddrinfo(result);

    return sockfd;
}

void *accepting(void *sockfd){

	int sockfd_conn;
    sockaddr_in peer_addr;
    socklen_t peer_addrlen = sizeof(peer_addr);

    memset(&peer_addr, 0, sizeof(peer_addr));

    sockfd_conn = accept(*(int *)sockfd, NULL, NULL); //non ci interessa l'IP di chi comunica con noi
	
	/*passo il controllo alla funzione che si occupa della comunicazione*/
    manager_requests(sockfd_conn);
}

void polling(int ListenSocket){
	int connessioni = 0;
	
	struct pollfd fds;
	fds.fd = ListenSocket;
	fds.events = POLLIN; //ci aspettiamo dati di normali e di priorità senza essere bloccati

	printf("\n\nIn attesa di connessioni...\n\n");
	
	pthread_mutex_init(&mutex, NULL); //inizializzo il mutex in manager_request.c
	
	while(1){
		memset(&(fds.revents), 0, sizeof(fds.revents));
		errno = 0;

		if(poll(&fds, 1, -1) == -1){
			if(errno == EINTR) printf("Segnale catturato durante il polling");
			return;
		}

		if((fds.revents & POLLHUP) || (fds.revents & POLLERR) || (fds.revents & POLLNVAL)){
			printf("Errore con il socket, disconnessione dal socket in corso...");
			return;
		}

		if(fds.revents & POLLIN){
			pthread_t thread_id;
            pthread_create(&thread_id, NULL, accepting, (void *)&ListenSocket);
		}
	}
}

void start_server(char *port_string){
	
	/*MAKE SOCKET AND BINDING---------------------------*/
	int socket = make_socket(port_string);
    
    /*LISTENING-----------------------------------------*/
    errno = 0;
	if(listen(socket, MAX_QUEUE) == -1){
		if(errno == EADDRINUSE) printf("Errore, un'altro socket è in ascolto sulla porta scelta\n");
		return;
	}
	
	polling(socket);
}