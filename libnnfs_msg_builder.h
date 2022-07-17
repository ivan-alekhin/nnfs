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
void build_fail_reply(struct MSG *, uint32_t, uint32_t, uint32_t, unsigned char *);