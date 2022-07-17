#include "libnnfs_queue.h"

void init_queue(struct queue_node **head){
    *head = NULL;
}

bool queue_is_empty(struct queue_node **head){
    return *head == NULL;
}

struct nnfs_context queue_pop(struct queue_node ** head){
    struct nnfs_context retval;
    
    if(queue_is_empty(head) != true){
        retval = (*head)->context;
        struct queue_node *buffer = * head;
        *head = (*head)->next;
        free(buffer);
        return retval;
    }
    else{
        exit(2);//easier to debug
    }
}

void queue_push(struct queue_node ** head, struct nnfs_context *client){
    struct queue_node * buffer = *head;
    *head = (struct queue_node *) calloc(1, sizeof(struct queue_node));
    (*head)->context = *client;
    (*head)->next = buffer;
}

void queue_clear(struct queue_node ** head){
    if(*head != NULL){
        queue_clear(&((*head)->next));
        free(*head);
    }
}