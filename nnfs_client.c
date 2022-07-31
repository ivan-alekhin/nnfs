#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "libnnfs_proto.h"
#include "libnnfs_msg_builder.h"
#include "libnnfs_socket.h"
#include "nnfs_constants.h"
#include "libnnfs_primitive_matcher.h"

const char* menu_text = "MENU:write one whole command, p.e. connect 0.0.0.0:24004\n1)connect ip_adress:port_number\n2)ping\n3)quit\n4)ls\n5)cd\n6)read\n7)write\n\n";
#define COMMAND_MAX_LENGTH 64u
#define CURDIR_LENGTH 512u
#define BUFFER_LENGTH 512u


void print_payload(const char * payload, const uint32_t payload_len){
    printf("%.*s", payload_len, payload);
}

//add proper debugging
int main(int argc, char *argv[]){

    // char curdir[CURDIR_LENGTH];
    // zeromem(curdir, CURDIR_LENGTH);

    char buffer[BUFFER_LENGTH + 1];
    char filename[BUFFER_LENGTH + 1];
    memset(buffer, 0, BUFFER_LENGTH + 1);
    memset(filename, 0, BUFFER_LENGTH + 1);

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
        memset(command, 0, strlen(command));
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
                build_quit_call(&message, ID++);
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
                
            case OP_CODE_LIST_DIRECTORY:
                build_ls_call(&message, ID++);
                nnfs_send(&context, &message);
                message.header.is_last = 0;
                printf("STATUS: CONTENTS OF THE DIRECTORY\n\n");
                while(message.header.is_last != 1){
                    nnfs_receive(&context, &message);
                    print_payload(message.payload, message.header.payload_len);
                }
                printf("\n\n");
                break;

            case OP_CODE_CHANGE_DIRECTORY:
                printf("Enter directory to go to:\n(use of . is prohibited and .. should only be in the start, ~ is considered home directory)\n");
                fgets(buffer, BUFFER_LENGTH, stdin);
                buffer[strlen(buffer) - 1] = 0;
                build_chdir_call(&message, ID++, buffer);
                nnfs_send(&context, &message);
                nnfs_receive(&context, &message);
                if(message.header.op_code == STATUS_FAIL_GARBAGE_ARGS){
                    printf("ERROR: invalid directory. It is %s\n", buffer);
                }
                if(message.header.op_code == STATUS_SUCCESS){
                    printf("SUCCESS: changed directory\n");
                }
                break;

            case OP_CODE_READ_FROM_REMOTE:
                printf("Enter filename to read from:\n");
                fgets(filename, BUFFER_LENGTH, stdin);
                filename[strlen(filename) - 1] = 0;
                build_read_file_call(&message, ID++, filename, READ_EVERYTHING, 0, READ_MODE_TEXT);
                printf("STATUS: sending read call, its payload_len is %d\n", message.header.payload_len);
                nnfs_send(&context, &message);
                message.header.is_last = 0;
                printf("CONTENTS OF A FILE:\n\n");
                while(message.header.is_last != 1){
                    nnfs_receive(&context, &message);
                    //printf("STATUS: op_code received = %d\n", message.header.op_code);
                    print_payload(message.payload, message.header.payload_len);
                    if(message.header.op_code == STATUS_FAIL_GARBAGE_ARGS)
                        printf("ERROR: file doesnt exist\n");
                }
                printf("\n\n");
                break;
            case OP_CODE_WRITE_FROM_LOCAL:
                printf("Enter filename to write to:\n");
                fgets(filename, BUFFER_LENGTH, stdin);
                filename[strlen(filename) - 1] = 0;

                printf("Enter text to write:\n");
                fgets(buffer, BUFFER_LENGTH, stdin);
                buffer[strlen(buffer) - 1] = 0;

                build_write_file_call(&message, ID++, filename, buffer);
                if(message.header.payload_len == 0){
                    printf("ERROR: too much to write at one time\n");
                    break;
                }
                nnfs_send(&context, &message);
                nnfs_receive(&context, &message);
                if(message.header.op_code == STATUS_SUCCESS){
                    printf("SUCCESS: successfully written to file\n");
                }
                else{
                    printf("ERROR: couldnt write to file, the reason is %d\n", message.header.op_code);
                }
                break;
            default:
                printf("ERROR: Invalid user command\n");
                break;
        }
    }
}