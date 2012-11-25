#ifndef NETWORK_H
#define NETWORK_H

#ifndef HOSTNAME
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de" //"localhost"
#endif
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
