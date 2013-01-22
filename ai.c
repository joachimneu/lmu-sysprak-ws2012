#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <limits.h>

#include "globals.h"

#include "ai.h"
#include "util.h"

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
					memcpy(&new_field, old_field, sizeof(struct field)); \
					new_field.field_data[fieldIndex(dx, dy)] = fieldValue(old_field, cx, cy); \
					new_field.field_data[fieldIndex(cx, cy)] = 1; \
					encodeMove(current_move, cx, cy, dx, dy); \
					*current_rating = 10; \
					nextMoveYield; \
				}

#define killIfPossible(methodX, methodY) \
				if(fieldExists(dx, dy)&&fieldIsOpponent(old_field, dx, dy)) { \
					int lx, ly; \
					lx = methodX(dx); ly = methodY(dy); \
					if(fieldExists(lx, ly)&&fieldEmpty(old_field, lx, ly)) { \
						struct field new_field; \
						memcpy(&new_field, old_field, sizeof(struct field)); \
						new_field.field_data[fieldIndex(lx, ly)] = fieldValue(old_field, cx, cy); \
						new_field.field_data[fieldIndex(cx, cy)] = 1; \
						new_field.field_data[fieldIndex(dx, dy)] = 1; \
						encodeMove(current_move, cx, cy, lx, ly); \
						*current_rating = 10; \
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
			int cf = fieldValue(old_field, cx, cy);
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
