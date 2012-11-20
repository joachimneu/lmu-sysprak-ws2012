#include <stdio.h>
#include <stdlib.h>

void die(char *string, int exit_code) {
	printf("Fatal error: %s\n", string);
	// hier muss aufgeräumt werden: Sockets, shared Memory, ...
	exit(exit_code);
}
