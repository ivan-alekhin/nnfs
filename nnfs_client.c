#include <stdio.h>
#include <stdbool.h>

#include "libnnfs_proto.h"
#include "libnnfs_msg_builder.h"
#include "libnnfs_socket.h"
#include "nnfs_constants.h"
#include "libnnfs_primitive_matcher.h"

const char* menu_text = "MENU:write one whole command, p.e. connect 0.0.0.0:24004\n1)connect ip_adress:port_number\n2)ping\n3)quit\n\n";
#define COMMAND_MAX_LENGTH 64u


//add proper debugging
int main(int argc, char *argv[]){

    uint32_t ID = STARTING_ID;
    struct nnfs_context context;
    nnfs_init_context(&context);

    struct ENCODED_MESSAGE encmes;
    init_encmes(&encmes);

    struct MSG message;
    init_msg(&message);

    char *command = calloc(1, COMMAND_MAX_LENGTH + 1);
    if(command == NULL){
        printf("ERROR: calloc in main client returned NULL\n");
        exit(1);
    }
    bool infinite_loop = true;
    uint32_t command_op_code = NULL_OP_CODE;
    int result= 0;

    //better commands now without the menu
    while(infinite_loop){
        printf("%s", menu_text);
        fgets(command, COMMAND_MAX_LENGTH, stdin);
        command_op_code = type_of_command(command);
        char *ip = NULL, *port = NULL;
        
        switch(command_op_code){
            case OP_CODE_CONNECT_CALL:
                //printf("%s\n", command);
                match_IPaddr(command, &ip);
                match_PORTnumber(command, &port);
                if(ip != NULL && port!=NULL){
                    printf("SUCCESS: connecting to %s at %s\n", port, ip);
                    result = nnfs_connect(&context, ip, port);
                } else{
                    result = 1;
                    if(port == NULL)
                        printf("ERROR: port should be separated by \":\" from IP: <ip>:<port>\n");
                    if(ip == NULL)
                        printf("ERROR: ip numbers should be divided by \".\": <number>.<number>.<number>.<number>\n");
                }
                free(ip);
                ip = NULL;
                free(port);
                port = NULL;
                if(result != 0)
                    printf("ERROR: couldnt connect to a remote server\n");
                //later a proper connect call will be added, there is no auth sequence right now, so its not needed
                //Ill work on it after the practice ends
                break;

            case OP_CODE_PING:
                build_ping_call(&message);
                result = nnfs_send(&context, &message);
                if(result == 0)
                    printf("ERROR: couldnt send a message\n");

                result = nnfs_receive(&context, &message);
                if(result == 0)    
                    printf("WARNING: received 0 bytes\n");

                if(message.header.op_code == STATUS_PONG)
                    printf("RESPONSE: PONG!\n");
                else
                    printf("ERROR: something went wrong in ping-pong exchange\n");
                break;

            case OP_CODE_CLOSE_CONNECTION:
                build_quit_call(&message, ID);
                result = nnfs_send(&context, &message);
                if(result == 0)
                    printf("ERROR: couldnt send a message\n");

                result = nnfs_receive(&context, &message);
                if(result == 0)    
                    printf("WARNING: received 0 bytes\n");

                if(message.header.op_code == STATUS_SUCCESS){
                    free(command);
                    printf("RESPONSE: Goodbye!\n");
                    result = nnfs_close(&context);
                    if(result != 0)
                        printf("ERROR: something went wrong while closing a connection\n");
                    return 0;
                } 
                else
                    printf("ERROR: something went wrong in the quit call\n");
                break;

            default:
                printf("ERROR: Invalid user command\n");
                break;
        }
    }
}