/** Rock Paper Scissors Server
 * Feb 25th 2019
 * Nick Fagan
 * B00767175
 */
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h> 
#include "rps.h"
#include "server.h"


/*** FUNCTION DECLARATIONS ***/
int handle(int socket);

/*** FUNCTION DEFINITIONS ***/

int main(int argc, char **argv) {
	srand(time(NULL));
	return run(argc, argv, handle);
}


int handle(int socket) {
	if(socket < 0) {
		return EXIT_FAILURE;
	}
	while(1) {
		enum Move p1_move;
		if(sizeof(enum Move) != read(socket, &p1_move, sizeof(enum Move))) {
			fprintf(stderr, "rpsserver: client disconnected.\n");
			break;
		}

		//validate Move
		if(! isValidMove(p1_move)) {
			fprintf(stderr, "rpsserver: client sent invalid move.\n");
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
	return EXIT_SUCCESS;
}
