#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <string.h>

#include "libnnfs_socket.h"
#include "libnnfs_msg_builder.h"
#include "nnfs_constants.h"
#include "libnnfs_primitive_matcher.h"
#include "libnnfs_queue.h"
#include "libnnfs_filesystem.h"

const char* server_menu = "MENU:write one whole command, p.e. bind 0.0.0.0:24004\n1)bind ip_address:port\n2)start max_clients\n3)setdir\n";

#define COMMAND_MAX_LENGTH 64u
#define MAX_NUMBER_OF_CLIENTS 16u

void zeromem(char * command, uint32_t length){
    for(uint32_t i = 0; i < length; i++){
        command[i] = '\0';
    }
}


//TODO: rework the client and the server
//1)add a quit sequence for both sides
//3)if possible move logging to a different terminal(may introduce more descriptive logs then)

void server_start(struct nnfs_context *server, uint16_t client_number);

int main(){
    uint32_t ID = STARTING_ID;
    struct nnfs_context server;
    nnfs_init_context(&server);

    DIR *valid_host_dir = NULL;
    int result = 0;
    char *command = calloc(1, COMMAND_MAX_LENGTH + 1);
    if(command == NULL){
        printf("ERROR: calloc in main server returned NULL\n");
        exit(1);
    }
    bool infinite_loop = true;
    uint32_t command_op_code = NULL_OP_CODE;
    char *ip = NULL, *port = NULL;
    bool hosting_dir_set = false;

    while(infinite_loop){
        printf("%s", server_menu);
        zeromem(command, strlen(command));
        fgets(command, COMMAND_MAX_LENGTH, stdin);
        command_op_code = type_of_command(command);
        switch(command_op_code){
            case SERVER_BIND:
                match_IPaddr(command, &ip);
                match_PORTnumber(command, &port);
                if(ip != NULL && port!=NULL){
                    printf("SUCCESS: binding to %s at %s\n", port, ip);
                    result = nnfs_bind(&server, ip, port);
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
                if(result != 0){
                    printf("ERROR: error binding. try changing port\n");
                }
                else
                    printf("SUCCESS: successful binding\n");
                break;
            
            case SERVER_LISTEN_AND_ACCEPT:
                result = match_client_number(command);
                if(result > 0 && result <= MAX_NUMBER_OF_CLIENTS && hosting_dir_set){
                    server_start(&server, result);
                }
                else{
                    if(hosting_dir_set)
                        printf("ERROR: invalid arguments -number of clients should be between 1 and %d\n", MAX_NUMBER_OF_CLIENTS);
                    else
                        printf("ERROR: hosting directory is invalid\n");
                }
                break;
            
            case SERVER_SET_DIRECTORY:
                printf("Write a path:\n");
                zeromem(command, strlen(command));
                fgets(command, COMMAND_MAX_LENGTH, stdin);
                command[strlen(command) -1] = 0;
                if(set_hosting_directory(command) == DIRECTORY_SUCCESS){
                    printf("SUCCESS: hosting directory is set\n");
                    printf("STATUS: checking validity...\n");
                    
                    valid_host_dir = opendir(command);
                    if(valid_host_dir != NULL){
                        printf("SUCCESS: hosting directory is valid\n");
                        closedir(valid_host_dir);
                        hosting_dir_set = true;

                    }
                    else{
                        printf("ERROR: hosting directory is invalid: %s\n", command);
                    }
                }
                else{
                    printf("ERROR: hosting directory was not set\n");
                }
                break;

            default:
                printf("ERROR: invalid user command\n");
                break;
        }    
    }
}

//my first attempt in threads will look wonky
//will put it into a different file later

//counting threads for debugging
atomic_char16_t counter = 0;


sem_t job_status;
pthread_mutex_t queue_mutex;
struct queue_node *head;
//only one thread changes this variable and once, therefore no protection is needed
bool shutting_down = false;

pthread_mutex_t filesystem_mutex;


//every call right now is only text
void respond_to_read_file(struct dir_descriptor *descr, struct nnfs_context *client, struct MSG *message, int thread_number){
    struct read_info rinfo;
    rinfo.file_path = NULL;
    printf("STATUS: making reading response\n");
    if(message->header.payload_len < NNFS_MSG_MAX_LENGTH && message->header.payload_len != 0){
        get_read_info_from_call(message->payload, &rinfo);
        printf("STATUS: the file contains name \"%s\"\n", rinfo.file_path);
        if(rinfo.read_mode != READ_MODE_TEXT){
            printf("ERROR: client tried reading in binary.\n");
            build_template_reply(message, message->header.ID, STATUS_FAIL_GARBAGE_ARGS,
                                    0, NULL, 1, 0);
            nnfs_send(client, message);
            free(rinfo.file_path);
            return;
        }
        printf("STATUS: client is reading file \"%s\"\n", rinfo.file_path);
        int offset = rinfo.offset;
        int sequence_number = 0;
        bool continue_reading = true;
        while(continue_reading){
            printf("STATUS: starting reading\n");
            build_success_reply(message, message->header.ID);
            if(rinfo.number_of_characters > NNFS_MSG_MAX_LENGTH || rinfo.number_of_characters == 0){
                printf("STATUS: entering if\n");
                rinfo.number_of_characters-= NNFS_MSG_MAX_LENGTH;
                message->header.payload_len = NNFS_MSG_MAX_LENGTH;
                message->header.number_in_sequence = sequence_number++;
                message->payload = calloc(1, message->header.payload_len);

                offset = read_from_file(descr, rinfo.file_path, message->payload, message->header.payload_len, offset);
                if(offset > 0)
                    message->header.is_last = 0;
                if(offset == 0)
                    continue_reading = false;
                if(offset < 0){
                    printf("ERROR: couldnt read from file\n");
                    build_template_reply(message, message->header.ID, STATUS_FAIL_GARBAGE_ARGS, 0, NULL, 1, sequence_number++);
                    continue_reading = false;
                }
                printf("STATUS: sending a string = \"%s\"\n", message->payload);
                nnfs_send(client, message);
            }
            else{
                printf("STATUS: entering else\n");
                message->header.is_last = 1;
                message->header.payload_len = rinfo.number_of_characters;
                message->payload = calloc(1, message->header.payload_len);
                offset = read_from_file(descr, rinfo.file_path, message->payload, message->header.payload_len, offset);
                message->header.number_in_sequence = sequence_number++;
                if(offset < 0){
                    printf("ERROR:couldnt read from file \"%s\"\n", rinfo.file_path);
                    build_template_reply(message, message->header.ID, STATUS_FAIL_GARBAGE_ARGS, 0, NULL, 1, sequence_number++);
                }
                printf("STATUS: sending a string = \"%s\"", message->payload);
                nnfs_send(client, message);
                continue_reading = false;
            }
        }
        free(rinfo.file_path);
    }
    else{
        printf("ERROR: client sent unsafe message payload length. It is %d\n", message->header.payload_len);
        build_template_reply(message, message->header.ID, STATUS_FAIL_UNSAFE_PAYLOAD_LENGTH,
                                0, NULL, 1, 0);
        nnfs_send(client, message); 
    }
}

void respond_to_write_to_file(struct dir_descriptor *descr, struct nnfs_context *client, struct MSG *message, int thread_number){
    struct write_info winfo;
    winfo.buffer = NULL;
    winfo.file_path = NULL;
    printf("STATUS: making write response\n");
    if(message->header.payload_len < NNFS_MSG_MAX_LENGTH && message->header.payload_len != 0){
        get_write_info_from_call(message->payload, message->header.payload_len, &winfo);
        int result = DIRECTORY_SUCCESS;
        if(winfo.buffer != NULL)
            result = write_to_file(descr, winfo.file_path, winfo.buffer);
        if(result != DIRECTORY_SUCCESS){
            printf("ERROR:couldnt write to file \"%s\"\n", winfo.file_path);
            build_template_reply(message, message->header.ID, STATUS_FAIL_GARBAGE_ARGS, 0, NULL, 1, 0);
        }
        else{
            printf("SUCCESS: wrote to a file\n");
            build_success_reply(message, message->header.ID);
        }
        nnfs_send(client, message);
        free(winfo.buffer);
        free(winfo.file_path);
    }
    else{
        printf("ERROR: client sent unsafe message payload length. It is %d\n", message->header.payload_len);
        build_template_reply(message, message->header.ID, STATUS_FAIL_UNSAFE_PAYLOAD_LENGTH,
                                0, NULL, 1, 0);
        nnfs_send(client, message); 
    }
}
//actual job the server will do
//case <constant>
//I believe that the <constant> name does the job at explaining what each case does
void workload(struct nnfs_context *client, int thread_number){
    bool infinite_loop = true;

    int dir_status = DIRECTORY_NOT_EVERYTHING_LISTED;
    int counter = 0;

    struct MSG message;
    init_msg(&message);
    struct dir_descriptor descr;
    create_new_dir_descriptor(&descr);
    char * buffer = NULL;

    while(infinite_loop){
        nnfs_receive(client, &message);
        switch(message.header.op_code){
            case OP_CODE_PING:
                printf("RESPONSE: thread %d is doing ping-pong message\n", thread_number);
                build_pong_reply(&message);
                nnfs_send(client, &message);
                break;

            case OP_CODE_CLOSE_CONNECTION:
                printf("RESPONSE: thread number %d is shutting down connection with the client: Client logged off\n", thread_number);
                build_success_reply(&message, message.header.ID);
                nnfs_send(client, &message);
                nnfs_shutdown(client);
                printf("SUCCESS: quit message has been sent\n");
                close_descriptor(&descr);
                infinite_loop = false;
                break;

            case OP_CODE_LIST_DIRECTORY:
                printf("RESPONCE thread number %d is listing directory\n", thread_number);
                buffer = calloc(1, NNFS_MSG_MAX_LENGTH);
                if(buffer == NULL){
                    printf("ERROR: couldnt calloc in server main\n");
                    exit(1);
                }
                counter = 0;
                dir_status = DIRECTORY_NOT_EVERYTHING_LISTED;
                rewind_directory(&descr);
                while(dir_status == DIRECTORY_NOT_EVERYTHING_LISTED){
                    dir_status = slist_directory(&descr, buffer, NNFS_MSG_MAX_LENGTH);
                    build_template_reply(&message, message.header.ID, STATUS_SUCCESS, (int)(strchr(buffer, '\0')- buffer), 
                                    buffer, !dir_status, counter);
                    nnfs_send(client, &message);
                    counter++;
                }
                free(buffer);
                buffer = NULL;
                break;

            case OP_CODE_CHANGE_DIRECTORY:
                if(message.header.payload_len < NNFS_MSG_MAX_LENGTH && message.header.payload_len != 0){
                    buffer = calloc(1, message.header.payload_len + 1);
                    if(buffer == NULL){
                        printf("ERROR: couldnt calloc in server main\n");
                        exit(1);
                    }
                    strncpy(buffer, message.payload, message.header.payload_len);
                    printf("RESPONCE thread number %d is changing directory to %s\n", thread_number, buffer);
                    dir_status = change_directory(&descr, buffer);
                    if(dir_status == DIRECTORY_SUCCESS){
                        printf("SUCCESS: client changed directory\n");
                        build_success_reply(&message, message.header.ID);
                    }
                    else{
                        printf("ERROR: client couldnt change directory to %s\n", buffer);
                        build_template_reply(&message, message.header.ID, STATUS_FAIL_GARBAGE_ARGS,
                                            0, NULL, 1, 0);
                    }
                    free(buffer);
                    buffer = NULL;
                }
                else{
                    printf("ERROR: client sent unsafe message payload length. It is %d\n", message.header.payload_len);
                    build_template_reply(&message, message.header.ID, STATUS_FAIL_UNSAFE_PAYLOAD_LENGTH,
                                        0, NULL, 1, 0);
                }
                nnfs_send(client, &message);
                break;

            case OP_CODE_READ_FROM_REMOTE:
                printf("STATUS: calling read respond function\n");
                respond_to_read_file(&descr, client, &message,thread_number);
                break;

            case OP_CODE_WRITE_FROM_LOCAL:
                printf("STATUS: calling write respond function\n");
                respond_to_write_to_file(&descr, client, &message, thread_number);
                break;

            default:
                build_template_reply(&message, message.header.ID, STATUS_FAIL_BAD_OP_CODE, 0, NULL, 1, 0);
                nnfs_send(client, &message);
                break;
        }
    }
}

void* acquiring_job(){
    //searching for a client
    //TODO:
    //pthread_mutex_lock(&counting_mutex);
    uint16_t thread_number = counter++;
    //pthread_mutex_unlock(&counting_mutex);

    struct nnfs_context client;
    while(!shutting_down){
        bool has_job = false;
        while(!has_job){
            sem_wait(&job_status);
            pthread_mutex_lock(&queue_mutex);
            printf("STATUS: Thread %d got a job\n", thread_number);               
            queue_pop(&head, &client);
            pthread_mutex_unlock(&queue_mutex);
            has_job = true;
        }
        workload(&client, thread_number);
    }
}

//right now there are no race conditions because every thread operates on its own client, so they have their own non-intersecting contexts
//every variable is different for every thread
//will be changed later after adding functionality that requires mutex
void server_start(struct nnfs_context *server, uint16_t client_number){

    bool working = true;
    uint32_t number_of_threads = client_number;
    pthread_t *threads = calloc(sizeof(pthread_t), client_number);
    if(threads == NULL){
        printf("ERROR: calloc in server_start returned NULL\n");
    }

    counter = 0u;
    init_queue(&head);
    struct nnfs_context client_buffer;
    sem_init(&job_status, 0, 0);
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_mutex_init(&filesystem_mutex, NULL);

    shutting_down = false;

    printf("STATUS: starting up...\n");
    for(int i = 0; i < client_number; i++){
        printf("STATUS: creating thread %d\n", i);
        if(pthread_create(threads + i, NULL,(void* (*)(void*)) &acquiring_job, NULL) != 0){
            printf("ERROR: couldnt create thread\n");
            exit(3);
        }
        printf("STATUS: created successfully\n");
    }
    nnfs_listen(server, client_number);
    printf("STATUS: started listening\n\n");
    //no need to join them because it will run until its stopped from outside
    while(!shutting_down){
        nnfs_accept(server, &client_buffer);
        printf("STATUS: client accepted\n");
        pthread_mutex_lock(&queue_mutex);
        queue_push(&head, &client_buffer);
        pthread_mutex_unlock(&queue_mutex);
        printf("STATUS: added client to the job pool. waiting to be picked up\n");
        sem_post(&job_status);
    }
    //unreachable code right now
    sem_destroy(&job_status);
    pthread_mutex_destroy(&queue_mutex);
    pthread_mutex_destroy(&filesystem_mutex);
    nnfs_close(server);
}