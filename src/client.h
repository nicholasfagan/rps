/* RPS client utils
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


/*** DEFINITIONS ***/

// multi-include protection
#ifndef RPSC_VER
#define RPSC_VER 1.0
#define RPSC_NAME "rpsclient"
#include "rps.h"



/*** STRUCTURES ***/

// Holds host and port for creating connection.
struct ClientConfig {
	char *host;
	int port;
};

// Holds socket file descriptor and servers remote address
struct Client {
	int sock;
	struct sockaddr_in addr;
};



/*** FUNCTION DECLARATIONS ***/

int run(int argc, char **argv, int (*handle)(int));

struct ClientConfig *createClientConfig(int argc, char **argv);
void destroyClientConfig(struct ClientConfig *config);

struct Client *createClient(struct ClientConfig *config);
void destroyClient(struct Client* client);

int connectClient(struct Client *client);
void disconnectClient(struct Client *client);

void usage(char *prgm);
void v_usage();



/*** FUNCTION DEFINITIONS ***/

/* Main logic for any client. 
 * Accepts command line arguments
 * and a function pointer 'int handle(int);'
 * to handle the created connection.
 *
 * returns the value from handle, or EXIT_FAILURE.
 *
 * int handle(int socket);
 *
 */
int run(int argc, char **argv, int (*handle)(int)) {
	//get the configuration
	struct ClientConfig *config = createClientConfig(argc, argv);
	if(config == NULL) {
		return EXIT_FAILURE;
	}

	//create connection
	struct Client *client = createClient(config);
	destroyClientConfig(config);//don't need config anymore.
	if(client == NULL) {
		return EXIT_FAILURE;
	}

	//attempt to connect
	if( connectClient(client) != EXIT_SUCCESS ) {
		destroyClient(client);
		return EXIT_FAILURE;
	}

	//we are now connected. handle.
	int return_value = handle(client->sock);

	//cleanup
	disconnectClient(client);
	destroyClient(client);

	return return_value;
}

/* Create a struct ClientConfig* from command line arguments.
 * On failure, returns NULL.
 */
struct ClientConfig *createClientConfig(int argc, char **argv) {
	struct ClientConfig *sc = NULL;
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
	} else if ( NULL == (sc = malloc(sizeof(struct ClientConfig))) ) {
		fprintf(stderr, "rpsclient: fatal error parsing arguments.\n");
	} else {
		sc->port = port;
		strcpy(hostname, argv[1]);
		sc->host = hostname;
	}
	return sc;
}

/* Destroys a non-NULL struct ClientConfig*
 */
void destroyClientConfig(struct ClientConfig *config) {
	if(config != NULL) {
		if(config->host != NULL) {
			free(config->host);
		}
		free(config);
	}	
}


/* Creates a struct Client* from a struct ClientConfig*
 * on failure, returns NULL.
 */
struct Client *createClient(struct ClientConfig *config) {
	if( config == NULL ) { 
		fprintf(stderr, "rpsclient: fatal error creating socket.\n");
		return NULL;
	}
	
	struct Client *client = NULL;
	
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
		} else if ( NULL == ( client = malloc(sizeof(struct Client)) ) ) {
			fprintf(stderr, "rpsclient: fatal error creating socket.\n");
		} else {
			client->addr = serv_addr;
			client->sock = sock;
		}
	}
	return client;
}

/* Destroys a non-NULL struct Client*
 */
void destroyClient(struct Client* client) {
	if(client != NULL) free(client);
}


/* Connect this struct Client*
 * EXIT_SUCCESS on success
 */
int connectClient(struct Client *client) {
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

/* Shuts down the socket of this connection.
 * disables any further communication.
 */
void disconnectClient(struct Client *client) {
	if (client != NULL) shutdown(client->sock, SHUT_RDWR);
}


// prints usage information to stderr.
void v_usage() {
	usage(RPSC_NAME);
}
void usage(char *prgm) {
	if ( prgm != NULL ) 
		fprintf(stderr, "rpsclient: usage: %s <hostname> <port>\n", prgm);
}


#endif
