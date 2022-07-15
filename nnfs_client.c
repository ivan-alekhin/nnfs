#include <stdio.h>
#include <stdbool.h>

#include "libnnfs_proto.h"
#include "libnnfs_msg_builder.h"
#include "libnnfs_socket.h"
#include "nnfs_message_codes.h"

const char* menu_text = "MENU:(use numbers right now, regex will be added later)\n1)(doesnt work right now)connect ip_adress port_number\n2)ping\n3)quit\n\n";
const char* IP = "0.0.0.0";
const char* PORT = "24001";

//add proper debugging
int main(int argc, char* argv[]){

    uint32_t ID = STARTING_ID;
    struct nnfs_context context;
    nnfs_init_context(&context);

    struct ENCODED_MESSAGE encmes;
    init_encmes(&encmes);

    struct MSG message;
    init_msg(&message);

    bool infinite_loop = true;
    char choice = 0;

    while(infinite_loop == true){
        printf("%s", menu_text);
        scanf(" %c", &choice);
        switch(choice){
            case '1':
                nnfs_connect(&context, IP, PORT);
                //later a proper connect call will be added, there is no auth sequence right now, so its not needed
                break;
            case '2':
                build_ping_call(&message);
                nnfs_send(&context, &message);
                nnfs_receive(&context, &message);
                if(message.header.op_code == STATUS_PONG)
                    printf("PONG!\n");
                break;
            case '3':
                build_quit_call(&message, ID);
                nnfs_send(&context, &message);
                nnfs_receive(&context, &message);
                if(message.header.op_code == STATUS_SUCCESS){
                    printf("Goodbye!\n");
                    nnfs_close(&context);
                    return 0;
                }
                break;
        }
    }
}
