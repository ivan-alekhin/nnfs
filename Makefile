CC=gcc
FLAGS=-c

all: client server

client: libnnfs_proto.o libnnfs_msg_builder.o libnnfs_socket.o nnfs_client.o libnnfs_primitive_matcher.o
	$(CC) nnfs_client.o libnnfs_socket.o libnnfs_proto.o libnnfs_primitive_matcher.o libnnfs_msg_builder.o -o nnfs_client

server:	libnnfs_proto.o libnnfs_msg_builder.o libnnfs_socket.o nnfs_client.o libnnfs_primitive_matcher.o libnnfs_queue.o nnfs_server.o libnnfs_filesystem.o
	$(CC) -pthread nnfs_server.o libnnfs_filesystem.o libnnfs_proto.o libnnfs_msg_builder.o libnnfs_socket.o libnnfs_primitive_matcher.o libnnfs_queue.o -o nnfs_server

test: utest_nnfs.o libnnfs_proto.o libnnfs_primitive_matcher.o
	$(CC) utest_nnfs.o libnnfs_proto.o libnnfs_primitive_matcher.o -o utest

filesystem_test: filesystem_test.c libnnfs_filesystem.o
	$(CC) $(FLAGS) filesystem_test.c
	$(CC) filesystem_test.o libnnfs_filesystem.o -o filesystem_test

libnnfs_filesystem.o: libnnfs_filesystem.c libnnfs_filesystem.h
	$(CC) $(FLAGS) libnnfs_filesystem.c

utest_nnfs.o: utest_nnfs.c
	$(CC) $(FLAGS) utest_nnfs.c

nnfs_server.o: nnfs_server.c
	$(CC) $(FLAGS) -pthread nnfs_server.c

nnfs_client.o: nnfs_client.c
	$(CC) $(FLAGS) nnfs_client.c

libnnfs_proto.o: libnnfs_proto.c libnnfs_proto.h
	$(CC) $(FLAGS) libnnfs_proto.c

libnnfs_msg_builder.o: libnnfs_msg_builder.c libnnfs_msg_builder.h
	$(CC) $(FLAGS) libnnfs_msg_builder.c

libnnfs_socket.o: libnnfs_socket.c libnnfs_socket.h
	$(CC) $(FLAGS) libnnfs_socket.c

libnnfs_primitive_matcher.o: libnnfs_primitive_matcher.c libnnfs_primitive_matcher.h
	$(CC) $(FLAGS) libnnfs_primitive_matcher.c

libnnfs_queue.o: libnnfs_queue.c libnnfs_queue.h
	$(CC) $(FLAGS) libnnfs_queue.c

clean:
	rm *.o