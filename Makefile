CC=g++

CFLAGS=-Wall -W -g -Werror 



all: client server

client: client.c raw.c connection_handler.c channels.c helper_functions.c
	$(CC) client.c raw.c connection_handler.c channels.c helper_functions.c $(CFLAGS) -o client -lpthread

server: server.c connection_handler.c channels.c helper_functions.c
	$(CC) server.c connection_handler.c channels.c helper_functions.c $(CFLAGS) -o server

clean:
	rm -f client server *.o
