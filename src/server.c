/** Rock Paper Scissors Server
 * Feb 25th 2019
 * Nick Fagan
 * B00767175
 */
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h> 
#include "rps.h"

typedef struct ServerConfig {
	unsigned int port;
} ServerConfig;

// prints usage information to stderr.
void usage(char *prgm);
void v_usage(void) {
	usage("rpsserver");
}
void usage(char *prgm) {
	if ( prgm != NULL ) 
		fprintf(stderr, "rpsserver: usage: %s <port>\n", prgm);
}

// Creates the configuration structure
// from command line arguments.
// returns null on failure, and prints the error to stderr.
ServerConfig* getServerConfig(int argc, char**argv);
ServerConfig* getServerConfig(int argc, char**argv) {
	ServerConfig *sc = NULL;
	int port = 0;
	if (argc != 2) {
		v_usage();
	} else if ( 1 != sscanf(argv[1], "%d", &port) ) {
		v_usage();
		fprintf(stderr, "rpsserver: <port> must be a number.\n");
	} else if ( port <= 0  || 65536 <= port ) {
		v_usage();
		fprintf(stderr, "rpsserver: <port> must be in range [1..65535]\n");
	} else if ( NULL != (sc = malloc(sizeof(ServerConfig))) ) {
		sc->port = port;
	} else {
		fprintf(stderr, "rpsserver: fatal error parsing arguments.\n");
	}
	return sc;
}



typedef struct Server {
	int file_desc;
	struct sockaddr_in address;
} Server;
// Creates socket FD, Sets socket options,
// binds the address, returns a structure containing
// FD and address.
// or null on failure.
Server* setupServer(ServerConfig* config);
Server* setupServer(ServerConfig* config) {
	if( config == NULL ) { 
		fprintf(stderr, "rpsserver: fatal error creating socket.\n");
		return NULL;
	}


	Server *server = NULL;
	int file_desc = -1;
	int option = 1;
	struct sockaddr_in saddr;

	if ( 0 == (file_desc = socket(AF_INET, SOCK_STREAM, 0)) ) {
		fprintf(stderr, "rpsserver: fatal error creating socket.\n");
	} else if ( EXIT_SUCCESS != 
			setsockopt(
				file_desc, 
				SOL_SOCKET, 
				SO_REUSEADDR | SO_REUSEPORT, 
				&option, sizeof(option)) ) {
		fprintf(stderr, "rpsserver: fatal error setting socket options.\n");
	} else {
		
		saddr.sin_port = htons(config->port); // set the port
		saddr.sin_family = AF_INET;          // use IPv4
		saddr.sin_addr.s_addr = INADDR_ANY;  // bind to any host

		if ( 0 > bind( file_desc, (struct sockaddr *)&saddr, sizeof(saddr) ) ) {
			fprintf(stderr, "rpsserver: fatal error binding socket.\n");
		} else if ( NULL == ( server = malloc(sizeof(Server)) ) ) {
			fprintf(stderr, "rpsserver: fatal error creating server.\n");
		} else {
			server->file_desc = file_desc;
			server->address = saddr;
		}
	}
	return server;
}

//Sets server to listen for connections.
//returns EXIT_SUCCESS on success or EXIT_FAILURE on failure.
int serverListen(Server* s);
int serverListen(Server* s) {
	if( s == NULL ) { 
		fprintf(stderr, "rpsserver: fatal error listening to socket.\n");
	} else if ( 0 > listen(s->file_desc, 3) ) {
		fprintf(stderr, "rpsserver: could not listen to socket.\n");
	} else {
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

// Accepts a clients connection.
// Returns the file descriptor of new socket, or <0 otherwise.
int serverAccept(Server* s);
int serverAccept(Server* s) {
	int clients_fd = -1;
	int address_len = sizeof(s->address);
	if ( 0 > 
			(clients_fd = accept(
										s->file_desc, 
										(struct sockaddr *)&(s->address),
										(socklen_t*)&address_len)) ) {
		fprintf(stderr,"rpsserver: fatal error accepting client.");
	} 
	return clients_fd;
}

// Accepts a client connection,
// calling f with the new socket.
// returns EXIT_FAILURE on failure, and f() on success.
int serverAcceptWith(Server *s, int (*f)(int));
int serverAcceptWith(Server *s, int (*f)(int)) {
	if(s == NULL) {
		fprintf(stderr, "rpsserver: fatal error with server.\n");
		return EXIT_FAILURE;
	}
	int cfd = -1;
	if(0 > (cfd = serverAccept(s))) {
		return EXIT_FAILURE;
	} else {
		return f(cfd);
	}
}



int handle(int socket) {
	while(1) {
		enum Move p1_move;
		if(sizeof(enum Move) != read(socket, &p1_move, sizeof(enum Move))) {
			fprintf(stderr, "rpsserver: client disconnected.\n");
			break;
		}
		int index = rand() % 3;
		enum Move p2_move = moves[index];
		enum Status outcome = whoWins(p1_move, p2_move);
		struct Game response = {
			.p1_move = p1_move,
			.p2_move = p2_move,
			.winner = outcome,
		};
		if (sizeof(struct	Game) != write(socket, &response, sizeof(struct Game))) {
			fprintf(stderr, "rpsserver: client disconnected.\n");
			break;
		}
		printGame(response);
	}
	return 0;
}
int main(int argc, char **argv) {
	srand(time(NULL));

	//set up the server
	ServerConfig *config;
	Server *server;
	if (         NULL == ( config = getServerConfig(argc, argv) ) 
	  ||         NULL == ( server = setupServer(config)         ) 
		|| EXIT_FAILURE == (   serverListen(server)               ) ){
		return EXIT_FAILURE;
	}
	free(config);

	// accept and handle connections.
	serverAcceptWith(server, &handle);

	shutdown(server->file_desc, SHUT_RDWR);
	free(server);

	return EXIT_SUCCESS;
}

