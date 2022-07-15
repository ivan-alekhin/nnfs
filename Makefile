CC=gcc
FLAGS=-c

all: client

client: libnnfs_proto.o libnnfs_msg_builder.o libnnfs_socket.o nnfs_client.o
	$(CC) nnfs_client.o libnnfs_socket.o libnnfs_proto.o libnnfs_msg_builder.o -o nnfs_client

nnfs_client.o: nnfs_client.c
	$(CC) $(FLAGS) nnfs_client.c

libnnfs_proto.o: libnnfs_proto.c libnnfs_proto.h
	$(CC) $(FLAGS) libnnfs_proto.c

libnnfs_msg_builder.o: libnnfs_msg_builder.c libnnfs_msg_builder.h
	$(CC) $(FLAGS) libnnfs_msg_builder.c

libnnfs_socket.o: libnnfs_socket.c libnnfs_socket.h
	$(CC) $(FLAGS) libnnfs_socket.c

clean:
	rm *.o
