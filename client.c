#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/prctl.h>

#include "util.h"
#include "network.h"
#include "ai.h"
#include "config.h"
#include "debug.h"

#define DEFAULT_CONFIG_FILE_NAME "client.conf"

int SOCKET = -1;
int PIPE[2];
struct game_state *GAME_STATE = (struct game_state *) -1;
enum whoami WHOAMI = THINKER;

void connector_handler_sigusr2() {
	// as our thinker defected to the enemy (we didn't expect that ...)
	// the connector has to clean the mess up ...
	cleanup();
	die("Oh no! They killed my father! Or did he kill himself? Who knows ...", EXIT_FAILURE);
}

void thinker_handler_sigusr1() {
	if(GAME_STATE->field_shmid != 0) {
		// read field from shared memory
		struct field *field = (struct field *) malloc(sizeof(struct field));
		char *serialized_field = (char *) shmat(GAME_STATE->field_shmid, NULL, 0);
		if(serialized_field == (char *) -1) {
			die("Could not attach shared memory!", EXIT_FAILURE);
		}
		fieldDeserialize(serialized_field, field);
		if(shmdt(serialized_field) == -1) {
			die("Could not detach shared memory!", EXIT_FAILURE);
		}
		if(shmctl(GAME_STATE->field_shmid, IPC_RMID, 0) == -1) {
			die("Could not remove shared memory!", EXIT_FAILURE);
		}
		
		// clear shared memory
		GAME_STATE->field_shmid = 0;
		
		// think
		think(field);
		
		// free field
		free(field->field_data);
		free(field);
	}
}

void usage(int argc, char *argv[]) {
	// how to use this program
	printf("USAGE: %s <gid> [<config>]\n", argv[0]);
	printf("  gid: 13-digit game-id without spaces\n");
	printf("  config (optional): configuration file, '" DEFAULT_CONFIG_FILE_NAME "' is assumed for default\n");
}

int main(int argc, char *argv[]) {
	int shmid = 0;
	pid_t tmp_pid;
	
	// "validate" first parameter: game id (gid)
	if(argc < 2 || argc > 3) {
		usage(argc, argv);
		die("Wrong number of parameters", EXIT_FAILURE);
	}
  if(strlen(argv[1]) != 13) {
	  usage(argc, argv);
	  die("The game id (gid) has to be exactly 13 digits!", EXIT_FAILURE);
  }
	
	// allocate shared memory for global GAME_STATE struct
	// (maybe we should not use 0x23421337 here as SHM key?)
	if((shmid = shmget(IPC_PRIVATE, sizeof(struct game_state), IPC_CREAT | IPC_EXCL | 0666)) < 0) {
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
	GAME_STATE->field_shmid = 0;

	// read configuration file (from 2nd argument if provided, o/w use default path)
	readConfig((argc==3)?argv[2]:DEFAULT_CONFIG_FILE_NAME);

	// open connection (i.e. socket + tcp connection)
	openConnection();
	// perform PROLOG phase of the protocol
	performConnection();

	// Create Pipe thinker<->connector
	if(pipe(PIPE)==-1) {
		die("Could not create pipe!", EXIT_FAILURE);
	}

	GAME_STATE->pid_thinker = getpid();
	tmp_pid = fork();
	if(tmp_pid < 0) {
		die("Could not fork for thinker/connector processes!", EXIT_FAILURE);
	} else if(tmp_pid == 0) { // child process = connector
		WHOAMI = CONNECTOR;
		
		// close pipe for writing
		close(PIPE[0]);
		
		// connect signal handler for SIGUSR2
		signal(SIGUSR2, connector_handler_sigusr2);
		// receive SIGUSR2 if parent (= thinker) dies
		prctl(PR_SET_PDEATHSIG, SIGUSR2);

		int move_duration = 0;
		while(1) {
			if((move_duration = handleLine())) {
				struct field *field;
				field = receiveField(SOCKET);
				sendTHINKING(SOCKET);
				
				// serialize data into shm
				if((GAME_STATE->field_shmid = shmget(IPC_PRIVATE, fieldSerializedSize(field), IPC_CREAT | IPC_EXCL | 0666)) < 0) {
					die("Could not get shared memory!", EXIT_FAILURE);
				}
				char *serialized_field = (char *) shmat(GAME_STATE->field_shmid, NULL, 0);
				if(serialized_field == (char *) -1) {
					// thinker will remove the shm in cleanup()
					die("Could not attach shared memory!", EXIT_FAILURE);
				}
				fieldSerialize(field, serialized_field);
				if(shmdt(serialized_field) == -1) {
					// thinker will remove the shm in cleanup()
					die("Could not detach shared memory!", EXIT_FAILURE);
				}
				free(field->field_data);
				free(field);
				
				// make thinker think
				kill(GAME_STATE->pid_thinker, SIGUSR1);
				
				expectOKTHINK(SOCKET);
				DEBUG("Oh gee, we have to think every now and then!!!\n");

				// Now we have to listen for new lines from the server and from the thinker simultaneously
				while(1) {
					fd_set* descriptors = NULL;
					FD_ZERO(descriptors);
					FD_SET(SOCKET, descriptors);
					FD_SET(PIPE[1], descriptors);
					select(2, descriptors, NULL, NULL, NULL); // waits until an element of 'descriptors' allows for nonblocking read operation (also stops on any signal)
					if(FD_ISSET(SOCKET, descriptors)) { // new line from Server (we don't expect any lines -> must be an error)
						dumpLine(SOCKET);
					}
					if(FD_ISSET(PIPE[1], descriptors)) { // Thinker done thinking
						char *buf = (char *) malloc((PROTOCOL_LINE_LENGTH_MAX-5+1) * sizeof(char));
						fscanf(fdopen(PIPE[1], "r"), "%[^\t\n]", buf);
						cmdPLAY(SOCKET, buf);
						free(buf);
						break;
					}
				}
			}
		}
	} else { // parent process = thinker
		GAME_STATE->pid_connector = tmp_pid;
		WHOAMI = THINKER;
		GAME_STATE->pid_thinker = getpid();

		// close pipe for reading
		close(PIPE[1]);

		// connect signal handler for SIGUSR1
		signal(SIGUSR1, thinker_handler_sigusr1);

		// wait for connector to finish
		int status;
		while(wait(&status) == -1) {
			if(status != 0) {
				break;
			}
		}
	}
	
	if(WHOAMI == THINKER) {
		cleanup();
	}
	return EXIT_SUCCESS;
}
