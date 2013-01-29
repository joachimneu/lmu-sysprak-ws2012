#include <stdio.h>
#include <string.h>
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
	// set shared memories for removal (to make as sure as possible it's removed ...)
	if(GAME_STATE != (struct game_state *) -1) {
		if(GAME_STATE->field_shmid > 0) {
			shmctl(GAME_STATE->field_shmid, IPC_RMID, 0);
		}
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

void fieldSerialize(struct field *f, char *dst) {
	int i;
	int *dst_ = (int *) dst;
	dst_[0] = f->width;
	dst_[1] = f->height;
	for(i = 0; i < f->width*f->height; i++) {
		dst_[i+2] = f->field_data[i];
	}
}

void fieldDeserialize(char *src, struct field *f) {
	int i;
	int *src_ = (int *) src;
	f->width = src_[0];
	f->height = src_[1];
	f->field_data = (int *) malloc(sizeof(int) * f->width*f->height);
	for(i = 0; i < f->width*f->height; i++) {
		f->field_data[i] = src_[i+2];
	}
}

int fieldSerializedSize(struct field *f) {
	return sizeof(int) * (f->width*f->height + 2);
}

void fieldPrint(struct field *f) {
	// TODO: something might be wrong ...
	int i,j;
	printf("X/Y\t|");
	for(j=0; j<f->width; j++) {
		printf("%c\t|", 65+j);
	}
	printf("\n");
	for(i=f->height-1; i>=0; i--) {
		printf("%i\t|", i+1);
		for(j=0; j<f->width; j++) {
			printf("%c(%c%s)\t|", ((i+j)%2==1)?'_':' ', 
					(f->field_data[i*f->width+j]<=1)?' ':(f->field_data[i*f->width+j]==3||f->field_data[i*f->width+j]==7)?'B':'W',
					(f->field_data[i*f->width+j]>=7)?"*":"");
		}
		printf("\n");
	}
}

struct field *fieldClone(struct field *g) {
	struct field *f = (struct field *) malloc(sizeof(struct field));
	int *field_data = (int *) malloc(sizeof(int) * g->width * g->height);
	memcpy(f, g, sizeof(struct field));
	f->field_data = field_data;
	memcpy(field_data, g->field_data, sizeof(int) * g->width * g->height);
	return f;
}

void fieldFree(struct field *f) {
	free(f->field_data);
	free(f);
}

void shortSerialize(short i, char dst[2]) {
	dst[0] = (i & 0x00FF) >> 0;
	dst[1] = (i & 0xFF00) >> 8;
}

short shortDeserialize(char dst[2]) {
	return dst[0] + (dst[1] << 8);
}
