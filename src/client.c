/** Rock Papar Scissors Client
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
#include <arpa/inet.h>
#include "rps.h"

typedef struct ClientConfig{
	unsigned int port;
	char *host;
} ClientConfig;

// prints usage information to stderr.
void usage(char *prgm);
void v_usage(void) {
	usage("rpsclient");
}
void usage(char *prgm) {
	if ( prgm != NULL ) 
		fprintf(stderr, "rpsclient: usage: %s <hostname> <port>\n", prgm);
}

// Creates the configuration structure
// from command line arguments.
// returns null on failure, and prints the error to stderr.
ClientConfig* getClientConfig(int argc, char**argv);
ClientConfig* getClientConfig(int argc, char**argv) {
	ClientConfig *sc = NULL;
	int port = 0;
	char *hostname = NULL;
	if (argc != 3) {
		v_usage();
	} else if (NULL == (hostname = malloc(sizeof(char)*strlen(argv[1])))) {
		fprintf(stderr, "rpsclient: fatal error parsing arguments.");
	} else if ( 1 != sscanf(argv[2], "%d", &port) ) {
		v_usage();
		fprintf(stderr, "rpsclient: <port> must be a number.\n");
	} else if ( port <= 0  || 65536 <= port ) {
		v_usage();
		fprintf(stderr, "rpsclient: <port> must be in range [1..65535]\n");
	} else if ( NULL == (sc = malloc(sizeof(ClientConfig))) ) {
		fprintf(stderr, "rpsclient: fatal error parsing arguments.\n");
	} else {
		sc->port = port;
		strcpy(hostname, argv[1]);
		sc->host = hostname;
	}
	return sc;
}



typedef struct Client {
	int sock;
	struct sockaddr_in addr;
} Client;
// Creates socket FD, Sets socket options,
// binds the address, returns a structure containing
// FD and address.
// or null on failure.
Client* setupClient(ClientConfig* config);
Client* setupClient(ClientConfig* config) {
	if( config == NULL ) { 
		fprintf(stderr, "rpsclient: fatal error creating socket.\n");
		return NULL;
	}
	
	Client *client = NULL;
	
	struct sockaddr_in serv_addr;
	int sock = -1;
	if (0 > (sock = socket(AF_INET, SOCK_STREAM, 0))) {
		fprintf(stderr,"rpsclient: could not create socket.\n");
	} else {

		memset(&serv_addr, '0', sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(config->port);

		if(0 >= inet_pton(AF_INET, config->host, &serv_addr.sin_addr)) {
			fprintf(stderr, "rpsclient: could not connect. invalid host/port.\n");
		} else if ( NULL == ( client = malloc(sizeof(Client)) ) ) {
			fprintf(stderr, "rpsclient: fatal error creating socket.\n");
		} else {
			client->addr = serv_addr;
			client->sock = sock;
		}
	}
	return client;
}


int connectClient(Client* client) {
	if (client == NULL) {
		fprintf(stderr, "rpsclient: fatal connection error.\n");
		return EXIT_FAILURE;
	} 
	else if (0 > connect(
				client->sock, 
				(struct sockaddr*)&(client->addr), 
				sizeof(client->addr))) {
		fprintf(stderr, "rpsclient: fatal: could not connect to server.\n");
		return EXIT_FAILURE;
	} 
	else {
		return EXIT_SUCCESS;	
	}
}


int handle(int socket) {
	while(1) {
		char c = 0;
		do {
			printf("Rock, Paper, or Scissors? (R|P|S): ");
			while((c = getchar())=='\n');
		} while(c != EOF && !isValidMoveChar(c));

		enum Move m = moveFromChar(c);
		if(sizeof(enum Move) != write(socket, &m, sizeof(enum Move))) {
			fprintf(stderr, "rpsclient: server disconnected.\n");
			break;
		}
		struct Game g;
		if(sizeof(struct Game) != read(socket, &g, sizeof(struct Game))) {
			fprintf(stderr, "rpsclient: server disconnected.\n");
			break;
		}
		
		printGame(g);
		do {
			printf("Would you like to continue? (Y/n)");
			while('\n' == (c = getchar()));
		} while (c != EOF && c != 'y' && c != 'Y' && c != 'n' && c != 'N' );
		if(c == EOF || c == 'n' || c == 'N') break;
	}
	return EXIT_SUCCESS;
}
int main(int argc, char **argv) {

	//set up the connection 
	ClientConfig *config;
	Client *client;
	if (         NULL == ( config = getClientConfig(argc, argv) ) 
	  ||         NULL == ( client = setupClient(config)         ) 
		|| EXIT_FAILURE == (   connectClient(client)              ) ){
		return EXIT_FAILURE;
	}
	
	free(config->host);free(config);

	//we are now connected. handle.
	handle(client->sock);
	shutdown(client->sock, SHUT_RDWR);

	free(client);

	return EXIT_SUCCESS;
}

