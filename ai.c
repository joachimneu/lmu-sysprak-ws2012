#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <limits.h>

#include "globals.h"

#include "ai.h"
#include "util.h"

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
						printf("gg %d %d\n", x, y);
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

// this is somewhat similar to "yield" in Python ... ;)
#define nextMoveYield { swapcontext(my_context, caller_context); }

#define moveLeftX(cx) (cx-1)
#define moveRightX(cx) (cx+1)
#define moveLeftY(cy) (player_id==1)?cy+1:cy-1
#define moveRightY(cy) (player_id==1)?cy+1:cy-1

#define fieldIndex(x, y) (y*8+x)
#define fieldValue(f, x, y) f->field_data[fieldIndex(x, y)]
#define fieldExists(x, y) (x>7||x<0||y>7||y<0)?false:true
#define fieldEmpty(f, x, y) (fieldValue(f, x, y)==1)?true:false
#define fieldIsOpponent(f, x, y) (player_id==1)?((fieldValue(f, x, y)==3||fieldValue(f, x, y)==7)?true:false):((fieldValue(f, x, y)==5||fieldValue(f, x, y)==9)?true:false)

#define moveIfPossible(methodX, methodY) \
				dx = methodX(cx); dy = methodY(cy); \
				if(fieldExists(dx, dy)&&fieldEmpty(old_field, dx, dy)) { \
					struct field new_field; \
					int *field_data; \
					field_data = (int *) malloc(sizeof(int) * old_field->width * old_field->height); \
					memcpy(&new_field, old_field, sizeof(struct field)); \
					new_field.field_data = field_data; \
					memcpy(field_data, old_field->field_data, sizeof(int) * old_field->width * old_field->height); \
					\
					new_field.field_data[fieldIndex(dx, dy)] = fieldValue(old_field, cx, cy); \
					new_field.field_data[fieldIndex(cx, cy)] = 1; \
					\
					encodeMove(current_move, cx, cy, dx, dy); \
					*current_rating = rateMove(&new_field); \
					nextMoveYield; \
				}

#define killIfPossible(methodX, methodY) \
				if(fieldExists(dx, dy)&&fieldIsOpponent(old_field, dx, dy)) { \
					int lx, ly; \
					lx = methodX(dx); ly = methodY(dy); \
					if(fieldExists(lx, ly)&&fieldEmpty(old_field, lx, ly)) { \
						struct field new_field; \
						int *field_data; \
						field_data = (int *) malloc(sizeof(int) * old_field->width * old_field->height); \
						memcpy(&new_field, old_field, sizeof(struct field)); \
						new_field.field_data = field_data; \
						memcpy(field_data, old_field->field_data, sizeof(int) * old_field->width * old_field->height); \
						\
						new_field.field_data[fieldIndex(lx, ly)] = fieldValue(old_field, cx, cy); \
						new_field.field_data[fieldIndex(cx, cy)] = 1; \
						new_field.field_data[fieldIndex(dx, dy)] = 1; \
						\
						encodeMove(current_move, cx, cy, lx, ly); \
						*current_rating = rateMove(&new_field); \
						nextMoveYield; \
					} \
				}

char encodeX(int x) {
	return "ABCDEFGH"[x];
}

char encodeY(int y) {
	return "12345678"[y];
}

void encodeField(char *dst, int x, int y) {
	dst[0] = encodeX(x);
	dst[1] = encodeY(y);
}

void encodeMove(char *dst, int cx, int cy, int dx, int dy) {
	encodeField(dst, cx, cy);
	dst[2] = ':';
	encodeField(dst+3, dx, dy);
	dst[5] = 0;
}

void nextMove(ucontext_t *my_context, ucontext_t *caller_context, int player_id, struct field *old_field, struct field *current_field, char *current_move, int *current_rating) {
	int cx = 0;
	int cy = 0;
	for(cy = 0; cy <= 7; cy++) {
		for(cx = 0; cx <= 7; cx++) {
			printf("%d ", fieldValue(old_field, cx, cy));
			
		}
		printf("\n");
	}
	for(cy = 0; cy <= 7; cy++) {
		for(cx = 0; cx <= 7; cx++) {
			int cf = fieldValue(old_field, cx, cy);
/*			printf("%d ", cf);*/
			int dx, dy;
			if((cf == 5 && player_id == 1) || (cf == 3 && player_id == 2)) {
				// Stein, correct player
				moveIfPossible(moveLeftX, moveLeftY)
				else killIfPossible(moveLeftX, moveLeftY)
				moveIfPossible(moveRightX, moveRightY)
				else killIfPossible(moveRightX, moveRightY)
			} else if(cf == 7 && player_id == 2) {
				// black Dame, black player
				
				*current_rating = 10;
/*				nextMoveYield;*/
			} else if(cf == 9 && player_id == 1) {
				// white Dame, white player
				
				*current_rating = 10;
/*				nextMoveYield;*/
			}
		}
/*		printf("\n");*/
	}
}

char *think(struct field *field) {
	fieldPrint(field);

	ucontext_t context_callee, context_resume, context_terminate;
	
	char iterator_stack[SIGSTKSZ];
	volatile int iterator_terminated;
	volatile struct field current_field;
	char *current_move = (char *) malloc(sizeof(char) * 512);
	volatile int current_rating = 0;
	char *best_move = (char *) malloc(sizeof(char) * 512);
	volatile int best_rating = INT_MIN;
	
	// copy current context into context_callee (iterator inherits signal settings, ...)
	// and set the context to switch to after termination of the iterator
	getcontext(&context_callee);
	context_callee.uc_link = &context_terminate;
	context_callee.uc_stack.ss_sp = iterator_stack;
	context_callee.uc_stack.ss_size = sizeof(iterator_stack);
	
	// initialize the context of the iterator
	makecontext(&context_callee, (void (*)(void)) nextMove, 7, &context_callee, &context_resume, GAME_STATE->own_player_id, field, &current_field, current_move, &current_rating);
	
	iterator_terminated = false;
	// after termination of the iterator, this context will be restored
	getcontext(&context_terminate);
	
	if(!iterator_terminated) {
		iterator_terminated = true;
		while(1) {
			swapcontext(&context_resume, &context_callee);
			printf("new move: %s (%d)\n", current_move, current_rating);
			if(current_rating > best_rating) {
				strcpy(best_move, current_move);
				best_rating = current_rating;
				printf("new best move: %s (%d)\n", best_move, best_rating);
			}
		}
	}
	
	return best_move;
}
