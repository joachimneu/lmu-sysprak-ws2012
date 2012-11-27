#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "util.h"
#include "globals.h"

void readConfig(char* file_name) {
  FILE* file; 
  char p_name[512];
  char *line = NULL;
  size_t linesize;
  if ((file = fopen(file_name, "r")) == NULL) {    
    die("Error! configuration file not found", EXIT_FAILURE);
  }
  
  while (getline(&line, &linesize, file) != -1) {
    // read parameter name
    sscanf(line, "%*[ ]%s%*[^ =]", p_name);
    
    // save parameter value
    if (strcmp(p_name, "hostname")) {    
      sscanf(line, "%*[^=]%*[ ]%512s%*[^ \n]", GAME_STATE->config_hostname);
    } else if (strcmp(p_name, "port")) {
      sscanf(line, "%*[^=]%*[ ]%i%*[^ \n]", &(GAME_STATE->config_port));
    } else if (strcmp(p_name, "gamekindname")) { 
      sscanf(line, "%*[^=]%*[ ]%512s%*[^ \n]", GAME_STATE->config_gamekindname);
    }    
    
  }
  free(line);
  fclose(file);
}
