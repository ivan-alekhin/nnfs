#include <stdio.h>
#include <stdbool.h>

#include "libnnfs_proto.h"
#include "libnnfs_msg_builder.h"
#include "libnnfs_socket.h"
#include "nnfs_message_codes.h"
#include "libnnfs_primitive_matcher.h"

const char* menu_text = "MENU:write whole command, p.e. connect 0.0.0.0:24004\n1)connect ip_adress:port_number\n2)ping\n3)quit\n\n";
#define COMMAND_MAX_LENGTH 64u


//add proper debugging
int main(int argc, char* argv[]){

    uint32_t ID = STARTING_ID;
    struct nnfs_context context;
    nnfs_init_context(&context);

    struct ENCODED_MESSAGE encmes;
    init_encmes(&encmes);

    struct MSG message;
    init_msg(&message);

    char * command = (char *) calloc(1, COMMAND_MAX_LENGTH + 1);
    bool infinite_loop = true;
    uint32_t command_op_code = NULL_OP_CODE;
    int result= 0;

    //better commands later without the menu
    while(infinite_loop == true){
        printf("%s", menu_text);
        fgets(command, COMMAND_MAX_LENGTH, stdin);
        command_op_code = type_of_command(command);
        char *ip = NULL, *port = NULL;
        
        switch(command_op_code){
            case OP_CODE_CONNECT_CALL:
                //printf("%s\n", command);
                match_IPaddr(command, &ip);
                match_PORTnumber(command, &port);
                printf("connecting to %s at %s\n", port, ip);
                result = nnfs_connect(&context, ip, port);
                if(result != 0)
                    printf("couldnt connect to a remote server\n");
                //later a proper connect call will be added, there is no auth sequence right now, so its not needed
                break;

            case OP_CODE_PING:
                build_ping_call(&message);
                nnfs_send(&context, &message);
                nnfs_receive(&context, &message);
                if(message.header.op_code == STATUS_PONG)
                    printf("PONG!\n");
                break;

            case OP_CODE_CLOSE_CONNECTION:
                build_quit_call(&message, ID);
                nnfs_send(&context, &message);
                nnfs_receive(&context, &message);
                if(message.header.op_code == STATUS_SUCCESS){
                    printf("Goodbye!\n");
                    nnfs_close(&context);
                    return 0;
                }
                break;

            default:
                printf("Invalid command\n");
                break;
        }
    }
}