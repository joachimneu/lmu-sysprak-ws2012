#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"

#include "ai.h"

char *think(struct field *field) {
/*	fieldPrint(field);*/
	
	int x, y;
	printf("think(): %i x %i\n", field->width, field->height);
	for(y = 0; y < field->height; y++) {
		printf("think(): ");
		for(x = 0; x < field->width; x++) {
			printf("%i ", field->field_data[y*field->height + x]);
		}
		printf("\n");
	}
	
	// TODO: ...
	char *result = malloc(sizeof(char) * 512);
	if(GAME_STATE->own_player_id == 1) {
		// I'm white ...
		strcpy(result, "E3:D4");
	} else if(GAME_STATE->own_player_id == 2) {
		// I'm black ...
		strcpy(result, "D6:C5");
	}
	return result;
}
