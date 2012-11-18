#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "util.h"
#include "network.h"

void usage(int argc, char *argv[]) {
	printf("BENUTZUNG: %s <Game-ID>\n", argv[0]);
	printf("  Game-ID: 13stellige Game-ID ohne Leerzeichen\n");
}

int main(int argc, char *argv[]) {
	int sock = 0;
	
	if(argc != 2) {
		usage(argc, argv);
		die("Fehler! Dieses Programm benötigt genau einen Parameter!", EXIT_FAILURE);
	}
	if(strlen(argv[1]) != 13) {
		usage(argc, argv);
		die("Fehler! Die Game-ID muss genau 13 Zeichen ohne Leerzeichen lang sein!", EXIT_FAILURE);
	}
	
	sock = openConnection();
	performConnection(sock, argv[1]);
	
	return EXIT_SUCCESS;
}
