#include "libnnfs_queue.h"

#include <stdio.h>
#include <assert.h>

void init_queue(struct queue_node **head){
    *head = NULL;
}

bool queue_is_empty(struct queue_node **head){
    return *head == NULL;
}

void queue_pop(struct queue_node **head, struct nnfs_context *retval){  
        assert(!queue_is_empty(head));  
        *retval = (*head)->context;
        struct queue_node *buffer = * head;
        *head = (*head)->next;
        free(buffer);
}

void queue_push(struct queue_node **head, struct nnfs_context *client){
    struct queue_node *buffer = *head;
    *head = calloc(1, sizeof(struct queue_node));
    if(*head == NULL){
        printf("ERROR: calloc in queue_push returned NULL\n");
        exit(1);
    }
    (*head)->context = *client;
    (*head)->next = buffer;
}

void queue_clear(struct queue_node ** head){
    struct queue_node *buffer;
    while(queue_is_empty(head)){
        buffer = *head;
        *head = (*head)->next;
        free(buffer);
    }
}