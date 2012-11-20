#ifndef GLOBALS_H
#define GLOBALS_H

struct game_state {
	char game_id[14];
	char game_name[512];
	int own_player_id;
	char own_player_color[512];
	int other_player_id;
	char other_player_color[512];
	int other_player_state;
};

extern int SOCKET;
extern struct game_state *GAME_STATE;

#endif
