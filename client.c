#include <stdio.h>
#include <stdbool.h>
#include "libnnfs_msg_builder.h"
#include "libnnfs_client_interface.h"

//right now it only hosts on a local machine
//I will change it after adding regex support to write proper commands
//as usual a lot of things in code may change, because Im not too happy with how it looks overall(right now it works, its possible to play ping pong with it)
//TODO: make a better menu

const char* menu_text = "MENU:(use numbers right now, regex will be added later)\n1)(doesnt work right now)connect ip_adress port_number\n2)ping\n3)quit\n\n";
const char* IP = "127.0.0.1";
#define MAX_USER_INPUT_LENGTH 16u

int main(int argc, char* argv[]){

    uint32_t ID = 1u;
    //will make it more consistent in syntax later
    struct nnfs_context client = nnfs_init_context();
    struct encoded_message encmes;
    init_encoded_message(&encmes);
    struct MSG message;
    init_MSG(&message);
    bool infinite_loop = true;
    char choice = 0;

    while(infinite_loop == true){
        printf("%s", menu_text);
        scanf(" %c", &choice);
        switch(choice){
            case '1':
                nnfs_connect(&client, IP, PORT);
                //later a proper connect call will be added, there is no auth sequence right now, so its not needed
                break;
            case '2':
                build_ping_call(&message);
                encode(message, &encmes);
                nnfs_send(&client, &encmes);
                nnfs_receive(&client, &encmes);
                decode(encmes, &message);
                if(message.op_code == STATUS_PONG)
                    printf("PONG!\n");
                break;
            case '3':
                destroymsg(message);
                build_quit_call(&message, ID);
                encode(message, &encmes);
                nnfs_send(&client, &encmes);
                nnfs_receive(&client, &encmes);
                decode(encmes,&message);
                if(message.op_code == STATUS_SUCCESS){
                    printf("Goodbye!\n");
                    nnfs_close(&client);
                    return 0;
                }
                break;
        }
    }
}
