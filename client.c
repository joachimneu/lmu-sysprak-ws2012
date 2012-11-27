#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "util.h"
#include "network.h"
#include "config.h"

#define DEFAULT_CONFIGURATION_FILE_NAME "client.conf"

int SOCKET = -1;
struct game_state *GAME_STATE = (struct game_state *) -1;

void usage(int argc, char *argv[]) {
	// how to use this program
	printf("USAGE: %s <gid> <<client.conf>>\n", argv[0]);
	printf("  gid: 13-digit game-id without spaces\n");
	printf("  client.conf (optional): configuration file, '" DEFAULT_CONFIGURATION_FILE_NAME "' is assumed for default\n");
}

int main(int argc, char *argv[]) {
	int shmid = 0;
	
	// "validate" first parameter: game id (gid)
	if(argc < 2 || argc > 3) {
		usage(argc, argv);
		die("Error! Wrong number of parameters", EXIT_FAILURE);
	}

  if(strlen(argv[1]) != 13) {
	  usage(argc, argv);
	  die("Error! The game id (gid) has to be exactly 13 digits!", EXIT_FAILURE);
  }
	
	// allocate shared memory for global GAME_STATE struct
	// (maybe we should not use 0x23421337 here as SHM key?)
	if((shmid = shmget(ftok("client.c", 0x23421337), sizeof(struct game_state), IPC_CREAT | IPC_EXCL | 0666)) < 0) {
		die("Could not get shared memory!", EXIT_FAILURE);
	}
	GAME_STATE = (struct game_state *) shmat(shmid, NULL, 0);
	if(GAME_STATE == (struct game_state *) -1) {
		// at this point our die() function can't figure out that the SHM was already
		// created, so we explicitly delete it here ...
		shmctl(shmid, IPC_RMID, 0);
		die("Could not attach shared memory!", EXIT_FAILURE);
	}
	GAME_STATE->shmid = shmid;
	strcpy(GAME_STATE->game_id, argv[1]);
	
	// read configuration
	readConfig((argc==3)?argv[2]:DEFAULT_CONFIGURATION_FILE_NAME);
	
	// open connection (i.e. socket + tcp connection)
	openConnection();
	// perform PROLOG phase of the protocol
	performConnection();

	GAME_STATE->pid_connector = getpid();
	if((GAME_STATE->pid_thinker = fork()) < 0) {
		die("Error! fork() did not work.", EXIT_FAILURE);
	}
	else if(GAME_STATE->pid_thinker == 0) { //Kindprozess = Thinker
		GAME_STATE->pid_thinker = getpid();
	}
	else { //Elternprozess = Connector
	}
		
	
	cleanup();
	return EXIT_SUCCESS;
}
