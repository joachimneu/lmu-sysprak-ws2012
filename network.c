#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "network.h"
#include "debug.h"

int _receiveLine(int sock, char **buf) {
	char result = 0;
	size_t length;
	getline(buf, &length, fdopen(sock, "r"));
	if((*buf)[0] == '+') { result = 1; } else { result = 0; }
	DEBUG("_receiveLine: %d '%s'\n", result, *buf);
	return result;
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

/*int line_recv(int fd, char *buf, int size) {*/
/*	printf("line_recv start\n");*/
/*	char c = 'A';*/
/*	int i = 0;*/
/*	while(recv(fd, &c, 1, 0)) {*/
/*		buf[i] = c;*/
/*		if(c == 10) {*/
/*			buf[i+1] = 0;*/
/*			printf("line_recv end 0\n");*/
/*			return 0;*/
/*		}*/
/*		i++;*/
/*	}*/
/*	printf("line_recv end 1\n");*/
/*	return 1;*/
/*}*/

void performConnection(int sock) {
	char *buf = NULL;
/*	printf("%s\n", recv*/
/*	send(sock, */
/*	send(fd, line, strlen(line), 0);*/
	printf("performing connection now ...\n");
	_receiveLine(sock, &buf);
	printf("{%s}\n", buf);
}
