#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"

#include "ai.h"
#include "util.h"

char *think(struct field *field) {
	fieldPrint(field);
	
	char *result = (char *) malloc(sizeof(char) * 512);
	if(GAME_STATE->own_player_id == 1) {
		// I'm white ...
		strcpy(result, "E3:D4");
	} else if(GAME_STATE->own_player_id == 2) {
		// I'm black ...
		strcpy(result, "D6:C5");
	}
	
	return result;
}
