#include "request.h"
#include "bacheca.c"
#include <sys/socket.h>
#include <stdbool.h>
#include <pthread.h>

pthread_mutex_t mutex;

void compatibility_check(int sockfd){
	char *string = "ciao 1";
	write(sockfd, string, sizeof(*string));
}

void send_options(int sockfd){
	char *string = "ciao 2";
	write(sockfd, string, sizeof(*string));
}

void send_all_messages(int sockfd){
	char *string = "ciao 3";
	write(sockfd, string, sizeof(*string));
}

void receive_message(int sockfd){
	char *string = "ciao 4";
	write(sockfd, string, sizeof(*string));
}

void authentication(int sockfd, bool authenticated, char * username){
	char *string = "ciao 5";
	write(sockfd, string, sizeof(*string));
}

void remove_message(int sockfd, bool authenticated, char * username){
	char *string = "ciao 6";
	write(sockfd, string, sizeof(*string));
}

bool manager_requests(int sockfd_conn){
	
	bool authenticated = false;
	char username[11]; //10+1

    while(1){
    	char req_type_str[2]; char **endptr;
    	int req_type;
    	REQ *request;

    	/*leggo la richiesta e preparo la struttura*/
    	recv(sockfd_conn, req_type_str, 1, MSG_PEEK);
		//inserire controllo se ancora connessi

    	req_type = strtol(req_type_str, endptr, 10);
    	if(**endptr != '\0') break;
		
    	switch(req_type){
    		case 0: compatibility_check(sockfd_conn); break;
    		case 1: send_options(sockfd_conn); break;
    		case 2: send_all_messages(sockfd_conn); break;
    		case 3: receive_message(sockfd_conn); break;
    		case 4: authentication(sockfd_conn, authenticated, username); break;
    		case 5: remove_message(sockfd_conn, authenticated, username); break;
    	}
	}
}