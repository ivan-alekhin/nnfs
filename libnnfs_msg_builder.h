#pragma once
#include "libnnfs_proto.h"

//builds connect call, ID is provided from outside
//right now the payload is empty
void build_connect_call(struct MSG *, uint32_t);

//builds reply on successful connection
//op_code is used as a STATUS in the reply message
void build_quit_call(struct MSG *, uint32_t);

//builds reply on FAILED connection
//op_code is used as a STATUS in the reply message
//the reason why connection failed will be stated in status and payload contains more information
void build_ping_call(struct MSG *);

//builds call to close connection
void build_success_reply(struct MSG *, uint32_t);

//ID for ping=pong is always 0, it is reserved
//is used to test if the client is still present
void build_pong_reply(struct MSG *);

//is used to respond to a ping call
void build_template_reply(struct MSG *, uint32_t ID, uint32_t reason_op_code, uint32_t payload_len, unsigned char *payload, char last_message, char seq_number);

//list directory call
void build_ls_call(struct MSG *, uint32_t ID);

//change directory call
void build_chdir_call(struct MSG *, uint32_t ID, const char * path);

//read from file
void build_read_file_call(struct MSG *message, uint32_t ID, const char *filename, uint32_t number_of_characters, uint32_t offset, uint32_t read_mode);

void build_write_file_call(struct MSG *message, uint32_t ID, const char *filename, const char *to_write);