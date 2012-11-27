#ifndef GLOBALS_H
#define GLOBALS_H

#include <unistd.h>

struct game_state {
	// SHM ID
	int shmid;
	
	// Game ID + Game Name
	char game_id[14];
	char game_name[512];
	
	// Player Informationen
	int num_player;
	int own_player_id;
	char own_player_color[512];
	int other_player_id;
	char other_player_color[512];
	int other_player_state;
	
	// Prozess-IDs
	pid_t pid_connector;
	pid_t pid_thinker;
};

extern int SOCKET;
extern struct game_state *GAME_STATE;

#endif
