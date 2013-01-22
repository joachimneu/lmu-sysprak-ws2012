#ifndef UTIL_H
#define UTIL_H

#include "globals.h"

#define sizeof_member(type, member) sizeof(((type *)0)->member)
#define false 0
#define FALSE 0
#define true 1
#define TRUE 1

void cleanup();
void die(char *, int);

void fieldSerialize(struct field *, char *);
void fieldDeserialize(char *, struct field *);
int fieldSerializedSize(struct field *);
void fieldPrint(struct field *);
void shortSerialize(short, char *);
short shortDeserialize(char *);

#endif
