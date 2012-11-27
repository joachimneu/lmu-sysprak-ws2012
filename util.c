#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "util.h"

void cleanup() {
	// close socket if necessary
	if(SOCKET != -1) {
		close(SOCKET);
	}
	// free GAME_STATE struct's space if necessary
	if(GAME_STATE != (struct game_state *) -1) {
		shmctl(GAME_STATE->shmid, IPC_RMID, 0);
	}
}

void die(char *string, int exit_code) {
	// message
	printf("Fatal error: %s\n", string);
	cleanup();
	// exit
	exit(exit_code);
}
