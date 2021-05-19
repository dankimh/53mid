CC=gcc
CFLAGS=-pthread -o

all: 
	$(CC) $(CFLAGS) client client.c
	$(CC) $(CFLAGS) server server.c
