CC=gcc
CFLAGS=-g -Wall
TARGET = client server
all : client server
client : client.c
	$(CC) -o $@ $<

server : server.c
	$(CC) -o $@ $<

clean:
	rm client server