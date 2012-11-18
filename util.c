#include <stdio.h>
#include <stdlib.h>

void die(char *string, int exit_code) {
	fprintf(stderr, "%s", string);
	exit(exit_code);
}
