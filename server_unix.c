#include "make_server.c"
#include <pthread.h>


int main(int argc, char* argv[]){

	if(argc < 2){
		printf("Usare: %s [port]\n",argv[0]);
		return 0;
	}
	
	start_server(argv[1]);
}
