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
	if(WHOAMI == CONNECTOR) {
		printf("[CONNECTOR] Fatal error: %s\n", string);
	} else if(WHOAMI == THINKER) {
		printf("[THINKER  ] Fatal error: %s\n", string);
	} else {
		printf("[UNKNOWN  ] Fatal error: %s\n", string);
	}
	if(WHOAMI == THINKER) {
		cleanup();
	}
	// exit
	exit(exit_code);
}


void fieldPrint(struct field *f) {
	int i,j;
	printf("X/Y\t|");
	for(j=0; j<f->width; j++) {
		printf("%c\t|", 65+j);
	}
	for(i=0; i<f->height; i++) {
		printf("%i\t|", f->height-i);
		for(j=0; j<f->width; j++) {
			printf("%c(%c%s)\t|", ((i+j)%2==0)?'W':'B', 
					(f->field_data[i*f->width+j]<=1)?'E':(f->field_data[i*f->width+j]==3||f->field_data[i*f->width+j]==7)?'S':'W',
					(f->field_data[i*f->width+j]>=7)?"*":"");
		}
		printf("\n");
	}
}

