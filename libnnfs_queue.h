#pragma once
#include <stdbool.h>

//making a queue for clients
#include "libnnfs_socket.h"
//we will build it once again

struct queue_node{
    struct nnfs_context context;
    struct queue_node *next; 
};

void init_queue(struct queue_node **head);
bool queue_is_empty(struct queue_node **head);
void queue_pop(struct queue_node ** head, struct nnfs_context *ptr);
void queue_push(struct queue_node **head, struct nnfs_context *context);
void queue_clear(struct queue_node **head);