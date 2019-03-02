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
	printf("Client Connected.\n");
	while(1) {
		enum Move p1_move;
		size_t len_rec = read(socket, &p1_move, sizeof(enum Move));
		if(len_rec == 0) {
			fprintf(stderr, "rpsserver: client disconnected.\n");
			break;
		}else if(sizeof(enum Move) != len_rec) {
			fprintf(stderr, "rpsserver: client sent malformed move.\n");
			break;
		}

		//validate Move
		if(! isValidMove(p1_move)) {
			fprintf(stderr, "rpsserver: client sent invalid move.\n");
			fprintf(stderr, "client sent 0x%X\n", p1_move);
			fprintf(stderr, "expected one of [0x%X, 0x%X, 0x%X]\n",Rock,Paper,Scissors);
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
