#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "globals.h"

void openConnection();
void performConnection();
int handleLine();

#endif
