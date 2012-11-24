#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "util.h"
#include "network.h"

int SOCKET = -1;
struct game_state *GAME_STATE = (struct game_state *) -1;

void usage(int argc, char *argv[]) {
	// how to use this program
	printf("USAGE: %s <gid>\n", argv[0]);
	printf("  gid: 13-digit game-id without spaces\n");
}

int main(int argc, char *argv[]) {
	// "validate" first parameter: game id (gid)
	if(argc != 2) {
		usage(argc, argv);
		die("Error! This program needs exactly one parameter!", EXIT_FAILURE);
	}
	if(strlen(argv[1]) != 13) {
		usage(argc, argv);
		die("Error! The game id (gid) has to be exactly 13 digits!", EXIT_FAILURE);
	}
	
	// allocate global GAME_STATE struct
	GAME_STATE = (struct game_state *) malloc(sizeof(struct game_state));
	strcpy(GAME_STATE->game_id, argv[1]);
	
	// open connection (i.e. socket + tcp connection)
	openConnection();
	// perform PROLOG phase of the protocol
	performConnection();
	
	return EXIT_SUCCESS;
}
