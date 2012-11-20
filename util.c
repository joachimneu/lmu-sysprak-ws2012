#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

void die(char *string, int exit_code) {
	printf("Fatal error: %s\n", string);
	if(SOCKET != -1) {
		close(SOCKET);
	}
	if(GAME_STATE != (struct game_state *) -1) {
		free(GAME_STATE);
	}
	exit(exit_code);
}
