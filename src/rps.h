/** Rock Paper Scissors Shared Functions
 * Feb 25th 2019
 * Nick Fagan
 * B00767175
 */
#include <stdlib.h>


/*** DEFINITIONS ***/
#ifndef RPS_VER
#define RPS_VER 1.0


/*** STRUCTURES ***/
enum Move {
	Rock = 0x3030,
	Paper = 0x3031,
	Scissors = 0x3032
};
enum Status {
	Tie,
	P1Win,
	P2Win,
};
struct Game {
	enum Move p1_move;
	enum Move p2_move;
	enum Status winner;
};

/*** CONSTANTS ***/
const enum Move moves[3] = {Rock, Paper, Scissors};

/*** FUNCTION DECLARATIONS ***/

char charFromMove(enum Move m);
enum Move moveFromChar(char m);
int isValidMoveChar(char m);
int isValidMove(enum Move m);
int isValidStatus(enum Status s);
int isValidGame(struct Game g);
enum Status whoWins(enum Move p1_move, enum Move p2_move);
void printMove(enum Move m);
void printStatus(enum Status s);
void printGame(struct Game g);

/*** FUNCTION DEFINITIONS ***/

char charFromMove(enum Move m) {
	switch(m) {
		case Rock:
			return 'R';
			break;
		case Paper:
			return 'P';
			break;
		case Scissors:
			return 'S';
			break;
	}
	return '\0';
}
enum Move moveFromChar(char m) {
	switch(m) {
		case 'R':
			return Rock;
			break;
		case 'P':
			return Paper;
			break;
		case 'S':
			return Scissors;
			break;
		default: 
			fprintf(stderr,"rps: invalid move\n");
			exit(EXIT_FAILURE);

	}
}

int isValidMoveChar(char m) {
	return m=='R'||m=='P'||m=='S';
}

int isValidMove(enum Move m) {
	return m==Rock||m==Paper||m==Scissors;
}

int isValidStatus(enum Status s) {
	return s==Tie||s==P2Win||s==P1Win;
}
int isValidGame(struct Game g) {
	return isValidMove(g.p1_move)
		&& isValidMove(g.p2_move)
		&& isValidStatus(g.winner);
}
enum Status whoWins(enum Move p1_move, enum Move p2_move) {
	if (p1_move == p2_move) return Tie;
	else if (p1_move == Rock) {
		if (p2_move == Paper) return P2Win;
		else return P1Win;
	}
	else if (p1_move == Paper) {
		if (p2_move == Scissors) return P2Win;
		else return P1Win;
	}
	else {
		if (p2_move == Rock) return P2Win;
		else  return P1Win;
	}
}

void printMove(enum Move m) {
	putchar(charFromMove(m));
}
void printStatus(enum Status s) {
	switch(s) {
		case P1Win:
			printf("Player 1 Wins");
		break;
		case P2Win:
			printf("Player 1 Wins");
		break;
		case Tie:
			printf("It's a Tie");
		break;
	}
}

void printGame(struct Game g) {
	printf("\nPlayer 1 chose ");
	printMove(g.p1_move);
	printf("\nPlayer 2 chose ");
	printMove(g.p2_move);
	printf("\n");
	printStatus(g.winner);
	printf("!\n");
}

#endif
