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

// Prints a human-readable version of the given field 
void fieldPrint(struct field *f) {
	int x,y;
	const int *d = f->field_data;

	printf("X/Y\t|");
	// 1. Row: column names (A,B,C,...)
	for(x=0; x < f->width; x++) {
		printf("%c\t|", 65+x);
	}
	printf("\n");

	// Print the rows of the field backwards
	for(y=f->height-1; y>=0; y--) {
		// 1. Column: row names (n, n-1,..., 2,1)
		printf("%i\t|", y+1);

		// Print state of each cell: f.i. '_(B*)' = Black cell (_) with black (B) queen (*) on it
		for(x=0; x<f->width; x++) {
			// Cell color: cell value = 0 -> white (always empty), o/w black
			printf("%c", (d[x + y*f->width] == 0)?' ':'_');
			// Cell values: 3 -> B, 5 -> W, 7 -> B*, 9 -> W*
			printf("(%c%s)\t|",
					(d[x + y*f->width] <= 1) ? ' ' : (d[x + y*f->width] == 3 || d[x + y*f->width] == 7) ? 'B' : 'W',
					(d[x + y*f->width] >= 7) ? "*" : "" );
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
