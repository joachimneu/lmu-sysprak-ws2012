#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

void die(char *string, int exit_code) {
	// message
	printf("Fatal error: %s\n", string);
	// close socket if necessary
	if(SOCKET != -1) {
		close(SOCKET);
	}
	// free GAME_STATE struct's space if necessary
	if(GAME_STATE != (struct game_state *) -1) {
		free(GAME_STATE);
	}
	// exit
	exit(exit_code);
}
