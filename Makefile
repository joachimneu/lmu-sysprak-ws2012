CC_EXE = gcc
CC_FLAGS = -Wall -Werror

CC = $(CC_EXE) $(CC_FLAGS)

all: client

clean:
	rm client *.o

client: client.c network.o debug.o util.o
	$(CC) -o client client.c network.o debug.o util.o

network.o: network.c network.h
	$(CC) -c network.c -o network.o

debug.o: debug.c debug.h
	$(CC) -c debug.c -o debug.o

util.o: util.c util.h
	$(CC) -c util.c -o util.o
