#include "libnnfs_socket.h"
#include "nnfs_constants.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

void nnfs_init_context(struct nnfs_context *context){
    context->socket = socket(AF_INET, SOCK_STREAM, 0);
};

int nnfs_connect(struct nnfs_context *client, const char *IP, const char *port){
    struct sockaddr_in server_adress;
    server_adress.sin_family = AF_INET;
    //temporary
    server_adress.sin_addr.s_addr = inet_addr(IP);
    server_adress.sin_port = htons(atoi(port));
    return connect(client->socket, (struct sockaddr *) &server_adress, sizeof(server_adress)); 
};

int nnfs_send(struct nnfs_context *client, struct MSG *message){
    struct ENCODED_MESSAGE encmes;
    init_encmes(&encmes);
    encode(message, &encmes);
    //printf("%d bytes are to be sent\n", encmes.length);
    int bytes_sent = send(client->socket, encmes.mes, encmes.length, 0);
    destroy_encmes(&encmes);
    if(ENABLE_LOGGING != 0)
        printf("SUCCESS: message sent\n");
    return bytes_sent;
};

int nnfs_receive(struct nnfs_context *context, struct MSG *message){
    struct ENCODED_MESSAGE encmes;
    encmes.mes = calloc(1, MSG_HEADER_SIZE);
    if(encmes.mes == NULL){
        printf("ERROR: calloc in nnfs_receive returned NULL\n");
        exit(1);
    }
    encmes.length = MSG_HEADER_SIZE;
        //printf("SUCCESS: %d bytes are to be recvd\n", encmes.length);
    int bytes_rcvd = recv(context->socket, encmes.mes, encmes.length, 0);
        //printf("SUCCESS: %d bytes are successfully recvd\n", encmes.length);
    decode_header(&encmes, message);
    destroy_encmes(&encmes);
    if(message->header.payload_len != 0){
        encmes.mes = calloc(1,message->header.payload_len);
        if(encmes.mes == NULL){
            printf("ERROR: calloc in nnfs_receive returned NULL\n");
            exit(1);
        }
        encmes.length = message->header.payload_len;
        bytes_rcvd += recv(context->socket, encmes.mes, encmes.length, 0);
        decode_payload(&encmes,message);
        destroy_encmes(&encmes);
    } 
    else{
        message->payload = NULL;
    }
    if(ENABLE_LOGGING != 0)
        printf("SUCCESS: message received\n");
    return bytes_rcvd;
};

int nnfs_shutdown(struct nnfs_context *client){
    return shutdown(client->socket, SHUT_RDWR);
};

int nnfs_close(struct nnfs_context *client){
    return close(client->socket);
};

int nnfs_bind(struct nnfs_context *context, const char *ip, const char *port){
    struct sockaddr_in adress;
    adress.sin_family = AF_INET;
    adress.sin_addr.s_addr = inet_addr(ip);
    adress.sin_port = htons(atoi(port));
    return bind(context->socket,(struct sockaddr *) &adress, sizeof(adress)); 
}

int nnfs_listen(struct nnfs_context *context, uint32_t max_clients){
    return listen(context->socket, max_clients);
}

int nnfs_accept(struct nnfs_context *context, struct nnfs_context *client){
    assert(client != NULL);
    client->socket = accept(context->socket, NULL, NULL);
    if (client->socket == -1) {
        return -errno;
    }
    if(ENABLE_LOGGING != 0)
        printf("SUCCESS: a client has made a TCP connection\n");
    return 0;
}
