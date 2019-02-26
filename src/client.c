/** Rock Papar Scissors Client
 * Feb 25th 2019
 * Nick Fagan
 * B00767175
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include "client.h"
#include "rps.h"

/*** FUNCTION DECLARATIONS ***/


int handle(int socket);

/*** FUNCTION DEFINITIONS ***/

int main(int argc, char **argv) {
	return run(argc, argv, &handle);
}

// this is whats passed to run() so that 
// when the connection is made,
// this function will be called
// with the socket.
int handle(int socket) {
	if(socket < 0) {
		fprintf(stderr,"rpsclient: could not handle connection.");
		return EXIT_FAILURE;
	}

	//main loop
	while(1) {

		char c = 0;
		do {// loop to keep asking until valid R S or P is given.
			printf("Rock, Paper, or Scissors? (R|P|S): ");
			while((c = getchar())=='\n');
		} while(c != EOF && !isValidMoveChar(c));

		//send the chosen move to the server
		enum Move m = moveFromChar(c);
		if(sizeof(enum Move) != write(socket, &m, sizeof(enum Move))) {
			fprintf(stderr, "rpsclient: server disconnected.\n");
			break;
		}

		//recieve the results
		struct Game g;
		if(sizeof(struct Game) != read(socket, &g, sizeof(struct Game))) {
			fprintf(stderr, "rpsclient: server disconnected.\n");
			break;
		}

		//validate
		if (! isValidGame(g)) {
			fprintf(stderr, "rpsclient: server sent invalid result.\n");
			break;
		}

		//print them
		printGame(g);
		
		// play again, conditional break.
		do { // loop until valid response
			printf("Would you like to continue? (y/n): ");
			while('\n' == (c = getchar()));
		} while (c != EOF && c != 'y' && c != 'Y' && c != 'n' && c != 'N' );
		if(c == EOF || c == 'n' || c == 'N') break;
	}

	// if we get here, either client closed the connection or server did.
	return EXIT_SUCCESS;
}

