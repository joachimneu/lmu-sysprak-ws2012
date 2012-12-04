#ifndef GLOBALS_H
#define GLOBALS_H

#include <unistd.h>

struct opponent {
	int id;
	char color[512];
	int state;
};

struct game_state {
	// SHM ID
	int shmid;
	
	// Game ID + Game Name
	char game_id[14];
	char game_name[512];
	
	// Player Informationen
	int num_players;
	int own_player_id;
	char own_player_color[512];
	struct opponent *opponents;

	// Process-IDs
	pid_t pid_connector;
	pid_t pid_thinker;

  // configuration file parameters
  char config_hostname[512];
  short config_port;
  char config_gamekindname[512];
};

enum whoami { CONNECTOR, THINKER };

extern int SOCKET;
extern struct game_state *GAME_STATE;
extern enum whoami WHOAMI;

#endif
