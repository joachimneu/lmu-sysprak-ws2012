#ifndef NETWORK_H
#define NETWORK_H

#define PROTOCOL_LINE_LENGTH_MAX 512

void openConnection();
void performConnection();
int handleLine();
void dumpLine(int);

struct field *receiveField(int);
void sendTHINKING(int);
void expectOKTHINK(int);
void cmdPLAY(int, char *);

#endif
