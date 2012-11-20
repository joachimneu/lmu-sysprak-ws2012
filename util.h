#ifndef UTIL_H
#define UTIL_H

#include "globals.h"

#define sizeof_member(type, member) sizeof(((type *)0)->member)

void die(char *, int);

#endif
