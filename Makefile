CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

all: server client

server: src/server.c
	$(CC) $(CFLAGS) $< -o $@

client: src/client.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f server client *.o

.PHONY: all clean
