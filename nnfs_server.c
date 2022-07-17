#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>

#include "libnnfs_socket.h"
#include "libnnfs_msg_builder.h"
#include "nnfs_message_codes.h"
#include "libnnfs_primitive_matcher.h"
#include "libnnfs_queue.h"

const char* server_menu = "MENU:write whole command, p.e. bind 0.0.0.0:24004\n1)bind ip_address:port\n2)start max_clients\n\n";
const char* IP_address = "0.0.0.0";
const char* port = "24005";
#define COMMAND_MAX_LENGTH 64u


//server is thoroughly untested
//but works fine in the conditions I tested it in
//all the printf are left because:
//1)Its likely not finished(not tested enough)
//2)the only life that a server can show without client is logging down everything
void server_start(struct nnfs_context * server, uint16_t client_number);

int main(){
    uint32_t ID = STARTING_ID;
    struct nnfs_context server;
    nnfs_init_context(&server);

    int result = 0;
    char * command = (char *) calloc(1, COMMAND_MAX_LENGTH + 1);
    bool infinite_loop = true;
    uint32_t command_op_code = NULL_OP_CODE;
    char *ip = NULL, *port = NULL;

    while(infinite_loop == true){
        printf("%s", server_menu);
        fgets(command, COMMAND_MAX_LENGTH, stdin);
        command_op_code = type_of_command(command);
        switch(command_op_code){
            case SERVER_BIND:
                match_IPaddr(command, &ip);
                match_PORTnumber(command, &port);
                printf("binding to %s at %s\n", port, ip);
                result = nnfs_bind(&server, ip, port);
                if(result != 0){
                    printf("error binding. try changing port\n");
                }
                else
                    printf("successful binding\n");
                break;
            
            case SERVER_LISTEN_AND_ACCEPT:
                result = match_client_number(command);
                if(result > 0){
                    server_start(&server, result);
                }
                else{
                    printf("ERROR: invalid arguments\n");
                }
                break;
            
            default:
                printf("invalid command\n");
                break;
        }    
    }
}

//my first attempt in threads will look wonky
//will put it into a different file later

//counting threads for debugging
pthread_mutex_t counting_mutex;
uint16_t counter;


pthread_mutex_t mutex;
struct queue_node *head;
bool job_available;
bool shutting_down;
void workload(struct nnfs_context * client, int thread_number){
    bool infinite_loop = true;
    struct MSG message;
    init_msg(&message);

    while(infinite_loop){
        nnfs_receive(client, &message);
        switch(message.header.op_code){
            case OP_CODE_PING:
                printf("thread %d is doing ping-pong message\n",thread_number);
                build_pong_reply(&message);
                nnfs_send(client, &message);
                break;
            case OP_CODE_CLOSE_CONNECTION:
                printf("thread number %d is shutting down connection with the client: Client logged off\n", thread_number);
                build_success_reply(&message, message.header.ID);
                nnfs_send(client, &message);
                nnfs_shutdown(client);
                infinite_loop = false;
                break;
            default:
                build_fail_reply(&message, message.header.ID, 0, 0, NULL);
                nnfs_send(client, &message);
                break;
        }
    }
}

void* acquiring_job(uint16_t thread_number){
    //searching for a client
    pthread_mutex_lock(&counting_mutex);
    thread_number = counter++;
    pthread_mutex_unlock(&counting_mutex);

    struct nnfs_context client;
    while(shutting_down != true){
        bool has_job = false;
        while(has_job != true){
            if(job_available == true){
                pthread_mutex_lock(&mutex);
                printf("thread %d noticed job_available flag is true\n", thread_number);
                if(queue_is_empty(&head) != true){
                    printf("job acquired by thread number %d\n", thread_number);
                    client = queue_pop(&head);
                    job_available = ! queue_is_empty(&head);
                    printf("thread %d started serving the client and job_available set to %d\n\n", thread_number, job_available);
                    has_job = true;
                }
                else{
                    printf("too bad for thread %d, the job is already occupied\n\n", thread_number);
                }
                pthread_mutex_unlock(&mutex);
            }
        }
        workload(&client, thread_number);
    }
}

//right now there are no race conditions because every thread operates on its own client, so they have their own non-intersecting contexts
//every variable is different for every thread
//will be changed later after adding functionality that requires mutex
void server_start(struct nnfs_context * server, uint16_t client_number){

    bool working = true;
    uint32_t number_of_threads = client_number;
    pthread_t* threads = (pthread_t *) calloc(sizeof(pthread_t), client_number);

    counter = 0u;
    init_queue(&head);
    struct nnfs_context client_buffer;
    pthread_mutex_init(&mutex, NULL);
    job_available = false;
    shutting_down = false;

    printf("starting up...\n");
    for(int i = 0; i < client_number; i++){
        printf("creating thread %d\n", i);
        if(pthread_create(threads + i, NULL,(void* (*)(void*)) &acquiring_job, (void *) &i) != 0){
            exit(3);
        }
        printf("created successfully\n");
    }
    nnfs_listen(server, client_number);
    printf("started listening\n\n");
    //no need to join them because it will run untill its stopped from outside
    while(!shutting_down){
        client_buffer = nnfs_accept(server);
        printf("client accepted\n");
        pthread_mutex_lock(&mutex);
        queue_push(&head, &client_buffer);
        job_available = true;
        printf("added client to the job pool\nwaiting to be picked up\n");
        pthread_mutex_unlock(&mutex);
    }

}