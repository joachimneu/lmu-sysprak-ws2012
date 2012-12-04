#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "util.h"

void cleanup() {
	// only thinker cleans shared ressources up ...
	if(WHOAMI == THINKER) {
		// close socket if necessary
		if(SOCKET != -1) {
			close(SOCKET);
		}
		// free GAME_STATE struct's space if necessary
		if(GAME_STATE != (struct game_state *) -1) {
			shmctl(GAME_STATE->shmid, IPC_RMID, 0);
		}
	}
}

void die(char *string, int exit_code) {
	// message
	if(WHOAMI == CONNECTOR) {
		printf("[CONNECTOR] Fatal error: %s\n", string);
	} else if(WHOAMI == THINKER) {
		printf("[THINKER  ] Fatal error: %s\n", string);
	} else {
		printf("[UNKNOWN  ] Fatal error: %s\n", string);
	}
	cleanup();
	// exit
	exit(exit_code);
}
