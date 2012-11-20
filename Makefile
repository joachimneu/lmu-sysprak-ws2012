CC_EXE = gcc
CC_FLAGS = -Wall -Werror -std=c99 -pedantic -D_POSIX_C_SOURCE=201211

CC = $(CC_EXE) $(CC_FLAGS)

client: client.c network.o debug.o util.o

clean:
	rm client *.o

%:
	$(CC) -o $@ $^

%.o: %.c %.h
	$(CC) -c $< -o $@
