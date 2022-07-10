#pragma once
#include "libnnfs_msg_builder.h"

#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#define PORT "24013"
//client interface to use in code


//struct that hides details
struct nnfs_context{
    int socket;

};

//inits the socket 
struct nnfs_context nnfs_init_context(){
    struct nnfs_context client;
    client.socket = socket(AF_INET, SOCK_STREAM, 0);
    return client;
}

//connects to a nnfs server
int nnfs_connect(struct nnfs_context *client, const char *IP, const char*port){
    struct sockaddr_in server_adress;
    server_adress.sin_family = AF_INET;
    //temporary
    server_adress.sin_addr.s_addr = INADDR_ANY;
    server_adress.sin_port = htons(atoi(port));
    return connect(client->socket, (struct sockaddr *) &server_adress, sizeof(server_adress)); 
}


//send a message to a remote server 
//returns how many bytes are sent
int nnfs_send(struct nnfs_context *client, struct encoded_message *message){
    return send(client->socket,(void*) message->mes, NNFS_MAX_LENGTH, 0);
}

//receive a message from a remote server
//returns how many bytes are received
int nnfs_receive(struct nnfs_context *client, struct encoded_message *message){
    return recv(client->socket, (void*) message->mes, NNFS_MAX_LENGTH, 0);
}

//shutdowns a nnfs_connection
//returns 0 on success
int nnfs_shutdown(struct nnfs_context *client){
    return shutdown(client->socket, SHUT_RDWR);
}

//closes a nnfs_socket
//may not free the socket adress for about 4 minutes, refer to the socket documentation
int nnfs_close(struct nnfs_context* client){
    return close(client->socket);
}
