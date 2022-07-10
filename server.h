#include "nnfs_client_interface.h"
#include <stdbool.h>
#include <stdio.h>

//right now it only hosts on a local machine
//I will change it after adding regex support to write proper commands
//as usual a lot of things in code may change, because Im not too happy with how it looks overall
//will try to do a dynamic port acquisition


int main(){
    uint32_t ID = 1u;
    struct nnfs_context server;
    server.socket = socket(AF_INET,SOCK_STREAM, 0);
    struct encoded_message encmes;
    init_encoded_message(&encmes);
    struct MSG message;
    init_MSG(&message);
    bool infinite_loop = true;
    
    struct sockaddr_in server_adress;
    server_adress.sin_family = AF_INET;
    server_adress.sin_addr.s_addr = INADDR_ANY;
    server_adress.sin_port = htons(atoi(PORT));

    if(bind(server.socket,(struct sockaddr*) &server_adress, sizeof(server_adress)) < 0){
        printf("error binding. try changing port in libnnfs_client_interface.h\n");
        exit(1);
    }
    listen(server.socket, 2u);
    struct nnfs_context client;
    client.socket = accept(server.socket, NULL, NULL);
    if(client.socket < 0){
        printf("error connecting\n");
        exit(1);
    }

    while(infinite_loop == true){
        nnfs_receive(&client, &encmes);
        decode(encmes, &message);
        if(message.op_code == OP_CODE_PING){
            destroymsg(message);
            build_pong_call(&message);
            encode(message, &encmes);
            nnfs_send(&client, &encmes);
        }

        if(message.op_code == OP_CODE_CLOSE_CONNECTION){
            destroymsg(message);
            build_success_reply(&message, message.ID);
            encode(message, &encmes);
            nnfs_send(&client,&encmes);

            infinite_loop = false;
            nnfs_shutdown(&client);
        }
    }
}
