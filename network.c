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
	} while((c != '\n') && (i < 512));
	// terminating 0-byte for the string
	buf[i] = 0;
	buf[strlen(buf)-1] = 0; // we're not interested in the \n ...*/
	DEBUG("_receiveLine: %d '%s'\n", (int) strlen(buf), buf);
	return buf;
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
	if((memcmp(buf, "+ PLAYING", 9) != 0) || (memcmp(buf+10, GAME_STATE->config_gamekindname, strlen(GAME_STATE->config_gamekindname) - 1) != 0)) {
		die("Gameserver sent wrong game kind!", EXIT_FAILURE);
	}
	DEBUG("We're playing '%s' as expected ...\n", GAME_STATE->config_gamekindname);
	free(buf);
	buf = recvLine(sock);
	// store the game name to the global GAME_STATE struct
	strncpy(GAME_STATE->game_name, &buf[2], sizeof_member(struct game_state, game_name));
	free(buf);
	DEBUG("Game name: '%s'\n", GAME_STATE->game_name);
}

void cmdPLAYER(int sock) {
	char *buf;
	int i;
	sendLine(sock, "PLAYER");
	// read my player id and player color
	buf = recvLine(sock);
	sscanf(buf, "+ YOU %i %s", &(GAME_STATE->own_player_id), GAME_STATE->own_player_color);
	free(buf);
	DEBUG("My player id is %i and my color is '%s'.\n", GAME_STATE->own_player_id, GAME_STATE->own_player_color);
	// read total number of players
	buf = recvLine(sock);
	sscanf(buf, "+ TOTAL %i", &(GAME_STATE->num_players));
	free(buf);
	DEBUG("There are %i players ...\n", GAME_STATE->num_players);
	GAME_STATE->opponents = (struct opponent *) malloc(sizeof(struct opponent) * (GAME_STATE->num_players -1));
	// read opponents player id, color and state
	for(i = 0; i < GAME_STATE->num_players-1; i++) {
		buf = recvLine(sock);
		sscanf(buf, "+ %i %s %i", &(GAME_STATE->opponents[i].id), GAME_STATE->opponents[i].color, &(GAME_STATE->opponents[i].state));
		free(buf);
		DEBUG("My #%i opponent's player id is %i and his color is '%s'.\n", i+1, GAME_STATE->opponents[i].id, GAME_STATE->opponents[i].color);
		if(GAME_STATE->opponents[i].state == 1) {
			DEBUG("My opponent is ready.\n");
		} else {
			DEBUG("My opponent is not yet ready.\n");
		}
	}
	expectLine(sock, "+ ENDPLAYERS");
}

void openConnection() {
	struct hostent *host_info;
	struct sockaddr_in server;
	// resolve gameserver hostname
	if((host_info = gethostbyname(GAME_STATE->config_hostname)) == NULL) {
		die("Could not resolve hostname of the gameserver!", EXIT_FAILURE);
	};
	// open socket
	if((SOCKET = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		die("Could not open socket!", EXIT_FAILURE);
	};
	memcpy( (char *)&server.sin_addr, host_info->h_addr_list[0], host_info->h_length );
	server.sin_port = htons(GAME_STATE->config_port);
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

int handleLine() {
	char *buf;
	buf = recvLine(SOCKET);

	// possible lines/commands to receive: '+ WAIT', '+ GAMEOVER' or '+ MOVE'
	if(strcmp(buf,"+ WAIT") == 0) {
		sendLine(SOCKET,"OKWAIT");
	} else if(strncmp(buf, "+ GAMEOVER ", 11) == 0) { // reads and prints winner id & color and quits
		int id_winner;
		char color_winner[512];
		sscanf(buf, "+ GAMEOVER %i %s", &id_winner, color_winner);
		printf("Gewinner hat ID: %i und Farbe: %s", id_winner, color_winner);
		fieldPrint(receiveField(SOCKET));
		die("GAMEOVER!", EXIT_SUCCESS);
	} else if(strncmp(buf,"+ MOVE ", 7) == 0) { // reads and returns duration of move, prints status message
		int move_duration;
		sscanf(buf,"+ MOVE %i ", &move_duration);
		free(buf);
		buf = recvLine(SOCKET);
		if(strncmp(buf, "+ STATUS ", 9) == 0) { // read status if any (might be "+ NOSTATUS" otherwise)
			printf("Status message: %s",buf+9);
		}
		free(buf);
		return move_duration;
	} else {
		die("Recieved an unspecified command from server!", EXIT_FAILURE);
	}
	free(buf);
	return 0;
}

struct field *receiveField(int sock) {
	struct field *f = malloc(sizeof(struct field));
	char *buf, *bufptr;
	int i, j, current_field, current_x, current_y;
	// "+ FIELD <width> <height>"
	buf = recvLine(sock);
	sscanf(buf, "+ FIELD %i,%i", &(f->width), &(f->height)); // receive width and height
	free(buf);
	f->field_data = (int *) malloc(sizeof(int) * f->width * f->height); // allocate space for the field data
	// read field rows: "+ <Y> <X_1,Y> <X_2,Y> ... <X_n,Y>"
	for(i = 0; i < f->height; i++) {
		buf = recvLine(sock);
		bufptr = buf;
		// read current y coordinate
		bufptr = strstr(bufptr, " ") + 1;
		sscanf(bufptr, "%i ", &current_y);
		current_y--;
		for(j = 0; j < f->width; j++) {
			// read fields ...
			current_x = j;
			bufptr = strstr(bufptr, " ") + 1;
			sscanf(bufptr, "%i ", &current_field);
			f->field_data[current_y * f->width + current_x] = current_field;
		}
		free(buf);
	}
	// "+ ENDFIELD"
	expectLine(SOCKET, "+ ENDFIELD");
	return f;
}

void sendTHINKING(int sock) {
	sendLine(sock, "THINKING");
}

void expectOKTHINK(int sock) {
	expectLine(sock, "+ OKTHINK");
}

void cmdPLAY(int sock, char *move) {
	sendLine(sock, "PLAY %s", move);
	dumpLine(sock);
}
