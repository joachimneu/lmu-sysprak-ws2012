CC_EXE = gcc
CC_FLAGS = -Wall -Werror

CC = $(CC_EXE) $(CC_FLAGS)

all: client

client: client.c network.o debug.o util.o
	$(CC) -o client client.c network.o debug.o

network.o: network.c
	$(CC) -c network.c -o network.o

debug.o: debug.c
	$(CC) -c debug.c -o debug.o

util.o: util.c
	$(CC) -c util.c -o util.o
