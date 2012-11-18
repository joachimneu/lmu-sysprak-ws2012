#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "network.h"
#include "util.h"
#include "debug.h"

void _receiveLine(int sock, char **buf) {
	size_t length;
	if(getline(buf, &length, fdopen(sock, "r")) <= 0) { // FEHLERBEHANDLUNG NOCHMAL CHECKEN!
		die("Could not read from socket, I guess network connection closed!", EXIT_FAILURE);
	}
	DEBUG("_receiveLine: %d %d '%s'\n", (int) length, (int) strlen(*buf), *buf);
}

void recvLine(int sock, char **buf) {
	_receiveLine(sock, buf);
	if((*buf)[0] != '+') { // FEHLERBEHANDLUNG NOCHMAL CHECKEN!
		printf("Gameserver Error: %s", buf[1]);
		die("Fatal gameserver error!", EXIT_FAILURE);
	}
}

void sendLine(int sock, const char* format, ...) {
	char buf[1024];
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buf, sizeof(buf), format, argptr);
	va_end(argptr);
	DEBUG("sendLine: %d '%s'\n", (int) strlen(buf), buf);
	send(sock, buf, strlen(buf), 0);
}

int openConnection() {
	struct hostent *host_info;
	struct sockaddr_in server;
	host_info = gethostbyname(HOSTNAME); // FEHLERBEHANDLUNG!
	int sock = socket(PF_INET, SOCK_STREAM, 0); // FEHLERBEHANDLUNG!
	memcpy( (char *)&server.sin_addr, host_info->h_addr, host_info->h_length );
	server.sin_port = htons(PORTNUMBER);
	server.sin_family = PF_INET;
	connect(sock, (struct sockaddr *) &server, sizeof(server)); // FEHLERBEHANDLUNG!
	return sock;
}

void performConnection(int sock, char *game_id) {
	char *buf = NULL;
	printf("performing connection now ...\n");
	buf = NULL; recvLine(sock, &buf); free(buf);
	sendLine(sock, "VERSION 1.0\n");
	buf = NULL; recvLine(sock, &buf); free(buf);
	sendLine(sock, "ID %s\n", game_id);
	buf = NULL; recvLine(sock, &buf); free(buf);
	buf = NULL; recvLine(sock, &buf); free(buf);
}
