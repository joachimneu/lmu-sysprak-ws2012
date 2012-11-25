#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "network.h"
#include "util.h"
#include "debug.h"

#define PROTOCOL_LINE_LENGTH_MAX 512

char *_receiveLine(int sock) {
	char *buf;
	int i = 0;
	char c = 0;
	// allocate space for the line to receive
	buf = (char *) malloc((PROTOCOL_LINE_LENGTH_MAX + 1) * sizeof(char));
	do {
		// read a byte
		if(read(sock, &c, 1) <= 0) {
			die("Could not read from socket, I guess the network connection died!", EXIT_FAILURE);
		}
		// store the byte
		buf[i] = c;
		// move on to the next index
		i++;
	// end when the received byte was a \n-byte
	} while(c != '\n');
	// terminating 0-byte for the string
	buf[i] = 0;
	buf[strlen(buf)-1] = 0; // we're not interested in the \n ...*/
	DEBUG("_receiveLine: %d '%s'\n", (int) strlen(buf), buf);
	return buf;
	
/*	// This old version of this function "ate" lines from time to time,*/
/*	// presumably because of the fdopen-stuff ...*/
/*	size_t length;*/
/*	FILE *fd;*/
/*	if((fd = fdopen(dup(sock), "r")) == NULL) {*/
/*		die("Could not duplicate socket for read, kernel issue!", EXIT_FAILURE);*/
/*	}*/
/*	if(getline(buf, &length, fd) <= 0) {*/
/*		die("Could not read from socket, I guess the network connection died!", EXIT_FAILURE);*/
/*	}*/
/*	fclose(fd);*/
/*	(*buf)[strlen(*buf)-1] = 0; // we're not interested in the \n ...*/
/*	DEBUG("_receiveLine: %d %d '%s'\n", (int) length, (int) strlen(*buf), *buf);*/
}

char *recvLine(int sock) {
	char *buf;
	// receive the line
	buf = _receiveLine(sock);
	// if the line is not positive, there's a fatal gameserver error
	if(buf[0] != '+') {
		printf("Gameserver Error: %s\n", buf);
		die("Fatal gameserver error!", EXIT_FAILURE);
	}
	return buf;
}

void sendLine(int sock, const char* format, ...) {
	char buf[1024];
	// format the line to send
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buf, sizeof(buf), format, argptr);
	va_end(argptr);
	DEBUG("sendLine: %d '%s'\n", (int) strlen(buf), buf);
	// send formated line + \n-byte
	if((send(sock, buf, strlen(buf), 0) < 0) || (send(sock, "\n", 1, 0) < 0)) {
		die("Could not write to socket, I guess the network connection died!", EXIT_FAILURE);
	}
}

void dumpLine(int sock) {
	// receive a line and dump that line immediately
	free(recvLine(sock));
}

void expectLine(int sock, char *line) {
	// receive a line and compare it to the given line
	char *buf = recvLine(sock);
	if(strcmp(buf, line) != 0) {
		die("I received an unexpected protocol line!", EXIT_FAILURE);
	}
	free(buf);
}

void cmdVERSION(int sock) {
	sendLine(sock, "VERSION 1.0");
	dumpLine(sock); // die when the server's negative about this version
}

void cmdID(int sock, char *game_id) {
	char *buf;
	sendLine(sock, "ID %s", game_id);
	buf = recvLine(sock);
	// check for right gamekind
	if(strcmp(buf, "+ PLAYING " GAMEKINDNAME) != 0) {
		die("Gameserver sent wrong game kind!", EXIT_FAILURE);
	}
	DEBUG("We're playing '" GAMEKINDNAME "' as expected ...\n");
	free(buf);
	buf = recvLine(sock);
	// store the game name to the global GAME_STATE struct
	strncpy(GAME_STATE->game_name, &buf[2], sizeof_member(struct game_state, game_name));
	free(buf);
	DEBUG("Game name: '%s'\n", GAME_STATE->game_name);
}

void cmdPLAYER(int sock) {
	char *buf;
	sendLine(sock, "PLAYER");
	buf = recvLine(sock);
	// read my player id and player color
	sscanf(buf, "+ YOU %i %s", &(GAME_STATE->own_player_id), GAME_STATE->own_player_color);
	free(buf);
	DEBUG("My player id is %i and my color is '%s'.\n", GAME_STATE->own_player_id, GAME_STATE->own_player_color);
	expectLine(sock, "+ TOTAL 2");
	DEBUG("As expected, we're two players.\n");
	buf = recvLine(sock);
	// read opponents player id and player color
	sscanf(buf, "+ %i %s %i", &(GAME_STATE->other_player_id), GAME_STATE->other_player_color, &(GAME_STATE->other_player_state));
	free(buf);
	DEBUG("My opponent's player id is %i and his color is '%s'.\n", GAME_STATE->other_player_id, GAME_STATE->other_player_color);
	if(GAME_STATE->other_player_state == 1) {
		DEBUG("My opponent is ready.\n");
	} else {
		DEBUG("My opponent is not yet ready.\n");
	}
	expectLine(sock, "+ ENDPLAYERS");
}

void openConnection() {
	struct hostent *host_info;
	struct sockaddr_in server;
	// resolve gameserver hostname
	if((host_info = gethostbyname(HOSTNAME)) == NULL) {
		die("Could not resolve hostname of the gameserver!", EXIT_FAILURE);
	};
	// open socket
	if((SOCKET = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		die("Could not open socket!", EXIT_FAILURE);
	};
	memcpy( (char *)&server.sin_addr, host_info->h_addr_list[0], host_info->h_length );
	server.sin_port = htons(PORTNUMBER);
	server.sin_family = PF_INET;
	// connect to gameserver
	if(connect(SOCKET, (struct sockaddr *) &server, sizeof(server)) < 0) {
		die("Could not connect to gameserver!", EXIT_FAILURE);
	}
}

void performConnection() {
	// throw away greetings
	dumpLine(SOCKET);
	
	// PROLOG phase of the protocol
	cmdVERSION(SOCKET);
	cmdID(SOCKET, GAME_STATE->game_id);
	cmdPLAYER(SOCKET);
}
