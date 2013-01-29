#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "util.h"
#include "debug.h"
#include "globals.h"

void readConfig(char *filename) {
	FILE* file;
	char p_name[512], p_value[512];
	char *line = NULL;
	char *lpos;
	size_t linesize;

	if((file = fopen(filename, "r")) == NULL) {
		die("Configuration file not found!", EXIT_FAILURE);
	}

	// default config
	strcpy(GAME_STATE->config_hostname, "sysprak.priv.lab.nm.ifi.lmu.de");
	GAME_STATE->config_port = 1357;
	strcpy(GAME_STATE->config_gamekindname, "Dame");

	// read parameters line by line
	while(getline(&line, &linesize, file) != -1) {
		lpos = line;
		// remove leading spaces
		while(isspace(*lpos)) lpos++;

		sscanf(lpos, "%[^ \t=]s", p_name); // read parameter name
		sscanf(lpos, "%*[^=]%*[ \t=]%512s%*[^ \t\n]", p_value); // read parameter value

		// save parameter
		if(strcmp(p_name, "hostname") == 0) {
			strcpy(GAME_STATE->config_hostname, p_value);
			DEBUG("Config: hostname = %s\n", GAME_STATE->config_hostname);
		} else if(strcmp(p_name, "port") == 0) {
			GAME_STATE->config_port = (unsigned short) atoi(p_value);
			DEBUG("Config: port = %i\n", GAME_STATE->config_port);
		} else if(strcmp(p_name, "gamekindname") == 0) {
			strcpy(GAME_STATE->config_gamekindname, p_value);
			DEBUG("Config: gamekindname = %s\n", GAME_STATE->config_gamekindname);
		}
	}

	free(line);
	fclose(file);
}
