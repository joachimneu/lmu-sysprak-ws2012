#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "debug.h"
#include "globals.h"

void readConfig(char *filename) {
	FILE* file;
	char p_name[512];
	char *line = NULL;
	size_t linesize;
	
	if((file = fopen(filename, "r")) == NULL) {
		die("Configuration file not found!", EXIT_FAILURE);
	}
	
	// default config
	strcpy(GAME_STATE->config_hostname, "sysprak.priv.lab.nm.ifi.lmu.de");
	GAME_STATE->config_port = 1357;
	strcpy(GAME_STATE->config_gamekindname, "Dame");
	
	while(getline(&line, &linesize, file) != -1) {
		// read parameter name
		p_name[0] = 0;
		sscanf(line, "%*[ ]%[^ =]s%*[ =]", p_name);
		if(p_name[0] == 0) {
			sscanf(line, "%[^ =]s%*[ =]", p_name);
		}
		
		// save parameter value
		if(strcmp(p_name, "hostname") == 0) {
			sscanf(line, "%*[^=]%*[ =]%512s%*[^ \n]", GAME_STATE->config_hostname);
			DEBUG("Config: hostname = %s\n", GAME_STATE->config_hostname);
		} else if(strcmp(p_name, "port") == 0) {
			sscanf(line, "%*[^=]%*[ =]%hi%*[^ \n]", &(GAME_STATE->config_port));
			DEBUG("Config: port = %i\n", GAME_STATE->config_port);
		} else if(strcmp(p_name, "gamekindname") == 0) {
			sscanf(line, "%*[^=]%*[ =]%512s%*[^ \n]", GAME_STATE->config_gamekindname);
			DEBUG("Config: gamekindname = %s\n", GAME_STATE->config_gamekindname);
		}
	}
	
	free(line);
	fclose(file);
}
