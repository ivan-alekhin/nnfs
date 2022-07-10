CC=gcc

all: server client
	
server: server.c
	$(CC) -g server.c -o nnfs_server

client: client.c
	$(CC) -g client.c -o nnfs_client
