#ifndef GLOBALS_H
#define GLOBALS_H

struct game_state {
	char game_id[14];
	char game_name[512];
};

extern int SOCKET;
extern struct game_state *GAME_STATE;

#endif
