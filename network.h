#ifndef NETWORK_H
#define NETWORK_H

#define HOSTNAME "localhost" //"sysprak.priv.lab.nm.ifi.lmu.de"
#define PORTNUMBER 1357
#define GAMEKINDNAME "Dame"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "globals.h"

void openConnection();
void performConnection();

#endif
