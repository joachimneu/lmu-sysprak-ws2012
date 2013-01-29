#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <limits.h>

#include "globals.h"

#include "ai.h"
#include "util.h"
#include "debug.h"

// this is somewhat similar to "yield" in Python ... ;)
#define nextMoveYield { swapcontext(my_context, caller_context); }

#define fieldAheadLeftX(cx) (cx-1)
#define fieldAheadRightX(cx) (cx+1)
#define fieldAheadLeftY(cy) ((player_id==1)?cy+1:cy-1)
#define fieldAheadRightY(cy) ((player_id==1)?cy+1:cy-1)
#define fieldBackLeftX(cx) (cx-1)
#define fieldBackRightX(cx) (cx+1)
#define fieldBackLeftY(cy) ((player_id==1)?cy-1:cy+1)
#define fieldBackRightY(cy) ((player_id==1)?cy-1:cy+1)

#define fieldIndex(x, y) (y*8+x)
#define fieldValue(f, x, y) (f->field_data[fieldIndex(x, y)])
#define fieldExists(x, y) ((x>7||x<0||y>7||y<0)?false:true)
#define fieldIsEmpty(f, x, y) ((fieldValue(f, x, y)==1)?true:false)
#define fieldIsOpponent(f, x, y) ((player_id==1)?((fieldValue(f, x, y)==3||fieldValue(f, x, y)==7)?true:false):((fieldValue(f, x, y)==5||fieldValue(f, x, y)==9)?true:false))

#define yieldMoveMoveIfPossible() { \
				if(fieldExists(dx, dy) && fieldIsEmpty(old_field, dx, dy)) { \
					current_field = fieldClone(old_field); \
					current_field->field_data[fieldIndex(dx, dy)] = cf; \
					current_field->field_data[fieldIndex(cx, cy)] = 1; \
					encodeMove(current_move, cx, cy, dx, dy); \
					*current_rating = rateMove(current_field, player_id); \
					nextMoveYield; \
					fieldFree(current_field); \
				} \
			}

#define yieldKillMoveIfPossible() { \
					if(fieldExists(dx, dy) && fieldExists(lx, ly) && fieldIsOpponent(old_field, dx, dy) && fieldIsEmpty(old_field, lx, ly)) { \
						current_field = fieldClone(old_field); \
						current_field->field_data[fieldIndex(lx, ly)] = cf; \
						current_field->field_data[fieldIndex(dx, dy)] = 1; \
						current_field->field_data[fieldIndex(cx, cy)] = 1; \
						encodeMove(current_move, cx, cy, lx, ly); \
						*current_rating = rateMove(current_field, player_id); \
						nextMoveYield; \
						fieldFree(current_field); \
					} \
				}

#define penaltyForBeingAboutToBeTakenStein(score_variable) penaltyForBeingAboutToBeTaken(score_variable, -70);
#define penaltyForBeingAboutToBeTakenDame(score_variable) penaltyForBeingAboutToBeTaken(score_variable, -250);

#define penaltyForBeingAboutToBeTaken(score_variable, points) { \
					if(fieldExists(ox, oy) && fieldExists(px, py) && fieldIsOpponent(f, ox, oy) && fieldIsEmpty(f, px, py)) { \
						score_variable += points; \
					} \
				}

#define sgn(x) ((x<0)?(-1):((x==0)?(0):(+1)))

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

bool straightLineIsFree(struct field *f, int src_x, int src_y, int dst_x, int dst_y) {
	int dx, dy, cx = src_x, cy = src_y;
	dx = sgn(dst_x - src_x);
	dy = sgn(dst_y - src_y);
	do {
		cx += dx; cy += dy;
		if(fieldValue(f, cx, cy) != 1) {
			return false;
		}
	} while(cx != dst_x && cy != dst_y);
	return true;
}

bool straightLineHasOnlyOneOpponent(struct field *f, int player_id, int src_x, int src_y, int dst_x, int dst_y, int *opp_x, int *opp_y) {
	int dx, dy, cx = src_x, cy = src_y;
	bool opponent_found = false;
	dx = sgn(dst_x - src_x);
	dy = sgn(dst_y - src_y);
	DEBUG("straightLineHasOnlyOneOpponent: %d %d %d %d %d %d\n", cx, cy, dst_x, dst_y, dx, dy);
	do {
		cx += dx; cy += dy;
		DEBUG("straightLineHasOnlyOneOpponent: %d %d %d\n", cx, cy, fieldValue(f, cx, cy));
		if(fieldValue(f, cx, cy) != 1) {
			if(fieldIsOpponent(f, cx, cy) && opponent_found == false) {
				// found the first opponent
				DEBUG("straightLineHasOnlyOneOpponent: first opponent found ...\n");
				*opp_x = cx;
				*opp_y = cy;
				opponent_found = true;
			} else if(fieldIsOpponent(f, cx, cy) && opponent_found == true) {
				// found a second opponent ...
				DEBUG("straightLineHasOnlyOneOpponent: unfortunately found a second opponent ...\n");
				return false;
			} else {
				// found one of my own Steins or Dames
				DEBUG("straightLineHasOnlyOneOpponent: found one of my own steins or dames ...\n");
				return false;
			}
		}
	} while(cx != dst_x && cy != dst_y);
	if(!opponent_found) {
		return false;
	}
	return true;
}

int rateMove(struct field *f, int player_id) {
	int white_score = 0;
	int black_score = 0;
	int x, y, ox, oy, px, py;
	
	for (y=0; y < (f->height); y++) {
		for (x=0; x < (f->width); x++) {
			switch (fieldValue(f, x, y)) {
				case 3: // black Stein
					// 100 points for having the Stein
					black_score += 100;
					
					// 2 points for each row closer to enemy home row (white's home row is at i=0)
					black_score += (f->height - y) * 2;
					
					// 1 point for most left / most right column
					if (x == 0 || (x+1) == f->width) {
						black_score += 1;
					}
					
					// 1000 points for protecting the home row
					// (we should never ever give it up as long as we can avoid it ...)
					if ((y+1) == f->height) {
						black_score += 1000;
					}
					
					// -70 points for being about to be taken (by enemy Stein)
					// ox = opponent x, oy = opponent y, px = protection x, py = protection y
					ox = fieldAheadLeftX(x); oy = fieldAheadLeftY(y); px = fieldBackRightX(x); py = fieldBackRightY(y);
					penaltyForBeingAboutToBeTakenStein(black_score);
					ox = fieldAheadRightX(x); oy = fieldAheadRightY(y); px = fieldBackLeftX(x); py = fieldBackLeftY(y);
					penaltyForBeingAboutToBeTakenStein(black_score);
					ox = fieldBackLeftX(x); oy = fieldBackLeftY(y); px = fieldAheadRightX(x); py = fieldAheadRightY(y);
					penaltyForBeingAboutToBeTakenStein(black_score);
					ox = fieldBackRightX(x); oy = fieldBackRightY(y); px = fieldAheadLeftX(x); py = fieldAheadLeftY(y);
					penaltyForBeingAboutToBeTakenStein(black_score);
					break;
				case 5: // white Stein
					// 100 points for having the Stein
					white_score += 100;
					
					// 2 points for each row closer to enemy home row (white's home row is at i=0)
					white_score += y * 2;
					
					// 1 point for most left / most right column
					if (x == 0 || (x+1) == f->width) {
						white_score += 1;
					}
					
					// 1000 points for protecting the home row
					// (we should never ever give it up as long as we can avoid it ...)
					if (y == 0) {
						white_score += 1000;
					}
					
					// -70 points for being about to be taken (by enemy Stein)
					// ox = opponent x, oy = opponent y, px = protection x, py = protection y
					ox = fieldAheadLeftX(x); oy = fieldAheadLeftY(y); px = fieldBackRightX(x); py = fieldBackRightY(y);
					penaltyForBeingAboutToBeTakenStein(white_score);
					ox = fieldAheadRightX(x); oy = fieldAheadRightY(y); px = fieldBackLeftX(x); py = fieldBackLeftY(y);
					penaltyForBeingAboutToBeTakenStein(white_score);
					ox = fieldBackLeftX(x); oy = fieldBackLeftY(y); px = fieldAheadRightX(x); py = fieldAheadRightY(y);
					penaltyForBeingAboutToBeTakenStein(white_score);
					ox = fieldBackRightX(x); oy = fieldBackRightY(y); px = fieldAheadLeftX(x); py = fieldAheadLeftY(y);
					penaltyForBeingAboutToBeTakenStein(white_score);
					break;
				case 7: // black Dame
					// 500 points for having a Dame
					black_score += 500;

					// -250 points for being about to be taken (by enemy Stein)
					// ox = opponent x, oy = opponent y, px = protection x, py = protection y
					ox = fieldAheadLeftX(x); oy = fieldAheadLeftY(y); px = fieldBackRightX(x); py = fieldBackRightY(y);
					penaltyForBeingAboutToBeTakenDame(black_score);
					ox = fieldAheadRightX(x); oy = fieldAheadRightY(y); px = fieldBackLeftX(x); py = fieldBackLeftY(y);
					penaltyForBeingAboutToBeTakenDame(black_score);
					ox = fieldBackLeftX(x); oy = fieldBackLeftY(y); px = fieldAheadRightX(x); py = fieldAheadRightY(y);
					penaltyForBeingAboutToBeTakenDame(black_score);
					ox = fieldBackRightX(x); oy = fieldBackRightY(y); px = fieldAheadLeftX(x); py = fieldAheadLeftY(y);
					penaltyForBeingAboutToBeTakenDame(black_score);
					break;
				case 9: // white Dame
					// 500 points for having a Dame
					white_score += 500;

					// -250 points for being about to be taken (by enemy Stein)
					// ox = opponent x, oy = opponent y, px = protection x, py = protection y
					ox = fieldAheadLeftX(x); oy = fieldAheadLeftY(y); px = fieldBackRightX(x); py = fieldBackRightY(y);
					penaltyForBeingAboutToBeTakenDame(white_score);
					ox = fieldAheadRightX(x); oy = fieldAheadRightY(y); px = fieldBackLeftX(x); py = fieldBackLeftY(y);
					penaltyForBeingAboutToBeTakenDame(white_score);
					ox = fieldBackLeftX(x); oy = fieldBackLeftY(y); px = fieldAheadRightX(x); py = fieldAheadRightY(y);
					penaltyForBeingAboutToBeTakenDame(white_score);
					ox = fieldBackRightX(x); oy = fieldBackRightY(y); px = fieldAheadLeftX(x); py = fieldAheadLeftY(y);
					penaltyForBeingAboutToBeTakenDame(white_score);
					break;
			}
		}
	}
	
	if(player_id == 1) {
		// I am white
		return white_score - black_score;
	} else {
		// I am black
		return black_score - white_score;
	}
}

void nextMove(ucontext_t *my_context, ucontext_t *caller_context, int player_id, struct field *old_field, struct field *current_field, char *current_move, int *current_rating) {
	int cx = 0;
	int cy = 0;
	for(cy = 0; cy <= 7; cy++) {
		for(cx = 0; cx <= 7; cx++) {
			int cf = fieldValue(old_field, cx, cy);
			int dx, dy, lx, ly;
			// cx = current x, cy = current y
			if((cf == 5 && player_id == 1) || (cf == 3 && player_id == 2)) {
				// Stein, correct player
				// dx = destination x, dy = destination y
				dx = fieldAheadLeftX(cx); dy = fieldAheadLeftY(cy);
				yieldMoveMoveIfPossible();
				dx = fieldAheadRightX(cx); dy = fieldAheadRightY(cy);
				yieldMoveMoveIfPossible();
				// dx = opponent x, dy = opponent y, lx = destination x, ly = destination y
				dx = fieldAheadLeftX(cx); dy = fieldAheadLeftY(cy); lx = fieldAheadLeftX(dx); ly = fieldAheadLeftY(dy);
				yieldKillMoveIfPossible();
				dx = fieldAheadRightX(cx); dy = fieldAheadRightY(cy); lx = fieldAheadRightX(dx); ly = fieldAheadRightY(dy);
				yieldKillMoveIfPossible();
				dx = fieldBackLeftX(cx); dy = fieldBackLeftY(cy); lx = fieldBackLeftX(dx); ly = fieldBackLeftY(dy);
				yieldKillMoveIfPossible();
				dx = fieldBackRightX(cx); dy = fieldBackRightY(cy); lx = fieldBackRightX(dx); ly = fieldBackRightY(dy);
				yieldKillMoveIfPossible();
			} else if((cf == 9 && player_id == 1) || (cf == 7 && player_id == 2)) {
				// Dame, correct player
				int i = 0, j = 0;
				for(j = 0; j < 2; j++) {
					for(i = -8; i <= 8; i++) {
						if(j == 1) {
							dx = cx + i; dy = cy - i;
						} else {
							dx = cx - i; dy = cy - i;
						}
						if(!fieldExists(dx, dy) || !fieldIsEmpty(old_field, dx, dy)) {
							continue;
						}
						// dx = destination x, dy = destination y
						if(straightLineIsFree(old_field, cx, cy, dx, dy)) {
							yieldMoveMoveIfPossible();
						} else {
							lx = dx;
							ly = dy;
							// dx = opponent x, dy = opponent y, lx = destination x, ly = destination y
							if(straightLineHasOnlyOneOpponent(old_field, player_id, cx, cy, lx, ly, &dx, &dy)) {
								yieldKillMoveIfPossible();
							}
						}
					}
				}
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
			DEBUG("new move: %s (%d)\n", current_move, current_rating);
			if(current_rating > best_rating) {
				strcpy(best_move, current_move);
				best_rating = current_rating;
				DEBUG("new best move: %s (%d)\n", best_move, best_rating);
			}
		}
	}
	
	return best_move;
}
