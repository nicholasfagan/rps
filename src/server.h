
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "rps.h"

/*** DEFINITIONS ***/
#ifndef RPSS_VER
#define RPSS_VER 1.0
#define RPSS_NAME "rpsserver"
#define RPSS_REP 0



/*** STRUCTURES ***/

struct ServerConfig {
	unsigned int port;
} ServerConfig;


struct Server {
	int sock;
	struct sockaddr_in addr;
} Server;

/*** FUNCTION DECLARATIONS ***/

int run(int argc, char **argv, int (*handle)(int));

struct ServerConfig *createServerConfig(int argc, char **argv);
void destroyServerConfig(struct ServerConfig *config);

struct Server* createServer(struct ServerConfig* config);
void destroyServer(struct Server* server);

int serverListen(struct Server* s);
int serverAccept(struct Server* s);

void closeServer(struct Server* s);

void usage(char *prgm);
void v_usage();


/*** FUNCTION DEFINITIONS ***/


int run(int argc, char **argv, int (*handle)(int)) {
	//set up the server
	struct ServerConfig *config = createServerConfig(argc, argv);
	if (NULL == config) {
		return EXIT_FAILURE;
	}
	struct Server *server = createServer(config);
	destroyServerConfig(config);
	if(NULL == server) {
		return EXIT_FAILURE;
	}
	if(EXIT_SUCCESS != serverListen(server)) {
		return EXIT_FAILURE;
	}

	int return_value;

	//continually accept connections?
	if( RPSS_REP ) {
		int client_socket;
		while(0 <= (client_socket = serverAccept(server)) ) {
			if (EXIT_SUCCESS != (return_value = handle(client_socket) )){
				break;
			}
		}
	} else {
		int client_socket = serverAccept(server);
		if(client_socket < 0) {
			return_value = EXIT_FAILURE;
		} else {
			return_value = handle(client_socket);
		}
	}

	closeServer(server);
	destroyServer(server);

	return return_value;
}

/* Creates  ServerConfig from ccommand line arguments, or NULL on failure.
 */
struct ServerConfig *createServerConfig(int argc, char **argv) {
	struct ServerConfig *sc = NULL;
	int port = 0;
	if (argc != 2) {
		v_usage();
	} else if ( 1 != sscanf(argv[1], "%d", &port) ) {
		v_usage();
		fprintf(stderr, "rpsserver: <port> must be a number.\n");
	} else if ( port <= 0  || 65536 <= port ) {
		v_usage();
		fprintf(stderr, "rpsserver: <port> must be in range [1..65535]\n");
	} else if ( NULL != (sc = malloc(sizeof(struct ServerConfig))) ) {
		sc->port = port;
	} else {
		fprintf(stderr, "rpsserver: fatal error parsing arguments.\n");
	}
	return sc;
}
/* Destroys a non-null config.
 */
void destroyServerConfig(struct ServerConfig *config) {
	if(config != NULL) free(config);
}

/* Creates a Server from a config,
 * or NULL on failure.
 */
struct Server* createServer(struct ServerConfig* config) {
	if( config == NULL ) { 
		fprintf(stderr, "rpsserver: fatal error creating socket.\n");
		return NULL;
	}


	struct Server *server = NULL;
	int sock = -1;
	int option = 1;
	struct sockaddr_in saddr;

	if ( 0 == (sock = socket(AF_INET, SOCK_STREAM, 0)) ) {
		fprintf(stderr, "rpsserver: fatal error creating socket.\n");
	} else if ( EXIT_SUCCESS != 
			setsockopt(
				sock, 
				SOL_SOCKET, 
				SO_REUSEADDR | SO_REUSEPORT, 
				&option, sizeof(option)) ) {
		fprintf(stderr, "rpsserver: fatal error setting socket options.\n");
	} else {
		
		saddr.sin_port = htons(config->port); // set the port
		saddr.sin_family = AF_INET;          // use IPv4
		saddr.sin_addr.s_addr = INADDR_ANY;  // bind to any host

		if ( 0 > bind( sock, (struct sockaddr *)&saddr, sizeof(saddr) ) ) {
			fprintf(stderr, "rpsserver: fatal error binding socket.\n");
		} else if ( NULL == ( server = malloc(sizeof(Server)) ) ) {
			fprintf(stderr, "rpsserver: fatal error creating server.\n");
		} else {
			server->sock = sock;
			server->addr= saddr;
		}
	}
	return server;
}
/* Destroys a Server
 */
void destroyServer(struct Server* server) {
	if(server != NULL) free(server);
}

/* Starts listening on the configured server.
 * returns EXIT_SUCCESS on success.
 */
int serverListen(struct Server* s) {
	if( s == NULL ) { 
		fprintf(stderr, "rpsserver: fatal error listening to socket.\n");
	} else if ( 0 > listen(s->sock, 3) ) {
		fprintf(stderr, "rpsserver: could not listen to socket.\n");
	} else {
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

/* Accepts a connection on the listening server
 */
int serverAccept(struct Server* s) {
	int clients_fd = -1;
	int address_len = sizeof(s->addr);
	if ( 0 > 
			(clients_fd = accept(
										s->sock, 
										(struct sockaddr *)&(s->addr),
										(socklen_t*)&address_len)) ) {
		fprintf(stderr,"rpsserver: fatal error accepting client.");
	} 
	return clients_fd;
}

void closeServer(struct Server* s) {
	shutdown(s->sock, SHUT_RDWR);
}

void usage(char *prgm) {
	fprintf(stderr, "rpsserver: usage: %s <port>", prgm);
}
void v_usage() {
	usage(RPSS_NAME);
}



#endif
