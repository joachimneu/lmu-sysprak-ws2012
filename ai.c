#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"

#include "ai.h"
#include "util.h"

char *think(struct field *field) {
	fieldPrint(field);
	
	char *result = (char *) malloc(sizeof(char) * 512);
	/*if(GAME_STATE->own_player_id == 1) {
		// I'm white ...
		strcpy(result, "E3:D4");
	} else if(GAME_STATE->own_player_id == 2) {
		// I'm black ...
		strcpy(result, "D6:C5");
	}*/
	
	// Mögliche Felder generieren
	// Felder bewerten
	// Beste Option zurückgeben
	
	return result;
}



int protectedLeft(int x, int y, struct field *f) {
	int *d = f->field_data;
	if (d[x*f->width + y] == 5 || d[x*f->width + y] == 9) { // White
		if (y==0 || x==0 || d[(x-1)*f->width + y-1] != 1) {
			return 1;
		}
	} else {
		if (y==0 || (x+1)==f->height || d[(x+1)*f->width + y-1] != 1) {
			return 1;
		}
	}
	return 0;
}

int protectedRight(int x, int y, struct field *f) {
	int *d = f->field_data;
	if (d[x*f->width + y] == 5 || d[x*f->width + y] == 9) { // White
		if ((y+1)==f->width || x==0 || d[(x-1)*f->width + y+1] != 1) {
			return 1;
		}
	} else {
		if ((y+1)==f->width || (x+1)==f->height || d[(x+1)*f->width + y+1] != 1) {
			return 1;
		}
	}
	return 0;
}


int rateMove(struct field *f) {
	int rating = 0, white_score = 0, black_score = 0, x, y;
	int *d = f->field_data;
	
	for (y=0; y<f->height; y++) {
		for (x=0; x<f->width; x++) {
			switch (d[x*f->height+y]) {
				case 3:
					black_score += 100; // 100 Points for having the piece
					black_score += (f->height-1-x)*2; // 2 Points for each row closer to enemy home row (whites home row is at i=0)
					if ((x+1)==f->height) { black_score += 500;} // 500 Points for protecting the home row
					if ( (y>0 && d[(x-1)*f->height + (y-1)] == 5 	&& !protectedLeft(x,y,f)) 
						|| ((y+1)<f->width && d[(x-1)*f->height + (y+1)] == 5 && !protectedRight(x,y,f))	) {
						black_score -= 70; // -90 Points for being about to be taken (by regular enemy piece)
					}
					break;
				case 5:
					white_score += 100; 
					white_score += x*2; 
					if (x==0) { white_score += 500;}
					if ( (y>0 && d[(x+1)*f->height+(y-1)] == 5 	&& !protectedLeft(x,y,f)) 
						|| ((y+1)<f->width && d[(x+1)*f->height + (y+1)] == 5 && !protectedRight(x,y,f))	) {
						black_score -= 90; // -90 Points for being about to be taken
					}
					break;
				case 7: // Black Queen
					black_score += 1000;
					break;
				case 9: // White Queen
					white_score += 1000;
					break;
			}
		}
	}
	
	if(GAME_STATE->own_player_id == 1) { // White
		rating = white_score - black_score;
	} else {
		rating = black_score - white_score;
	}
	
	return rating;
}






















