#pragma once
#include "libnnfs_proto.h"

//struct that hides details
struct nnfs_context{
    int socket;
};

//inits the socket 
void nnfs_init_context(struct nnfs_context *);

//connects to a nnfs server
int nnfs_connect(struct nnfs_context *, const char *ip, const char*port);

//binds socket to the address and port
int nnfs_bind(struct nnfs_context *, const char *ip, const char *port);

//starts listening for clients
int nnfs_listen(struct nnfs_context *, uint32_t max_clients);

//accepts connection returning client information
void nnfs_accept(struct nnfs_context *server, struct nnfs_context *client);

//send a message to a remote server 
//returns how many bytes are sent
int nnfs_send(struct nnfs_context *, struct MSG *);

//receive a message from a remote server
//returns how many bytes are received
int nnfs_receive(struct nnfs_context *, struct MSG *);

//shutdowns a nnfs_connection
//returns 0 on success
int nnfs_shutdown(struct nnfs_context *);

//closes a nnfs_socket
//may not free the socket adress for about 4 minutes, refer to the socket documentation
int nnfs_close(struct nnfs_context *);

