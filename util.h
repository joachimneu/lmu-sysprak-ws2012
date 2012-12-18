#ifndef UTIL_H
#define UTIL_H

#include "globals.h"

#define sizeof_member(type, member) sizeof(((type *)0)->member)

void cleanup();
void die(char *, int);

void fieldSerialize(struct field *, char *);
void fieldDeserialize(char *, struct field *);
int fieldSerializedSize(struct field *);

#endif
