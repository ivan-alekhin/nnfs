#pragma once
#include <string.h>
#include "libnnfs_proto.h"

//library's purpose is to introduce instruction to values of MSG.op_code
//numbers are chosen on random, may change in the future
//NULL_OP_CODE will be useful in troubleshooting, it represents no value
#define NULL_OP_CODE 256u 
#define OP_CODE_CONNECT_CALL 257u
#define OP_CODE_CLOSE_CONNECTION 258u
#define OP_CODE_PING 259u

//not implemented op_codes:
#define OP_CODE_LIST_DIRECTORY 259u
#define OP_CODE_CHANGE_DIRECTORY 260u


//op_code in TYPE_REPLY represents status
#define STATUS_SUCCESS 0u
#define STATUS_FAIL 1u
#define STATUS_PONG 2u

//defines a basic template call
void inner_build_template_call(struct MSG *message, uint32_t ID){
    message->type = TYPE_CALL;
    message->ID = ID;
    message->op_code = NULL_OP_CODE;
    message->payload_len = 0u;
    message->payload = NULL;
}

//defines a basic template reply
void inner_build_template_reply(struct MSG *message, uint32_t ID){
    message->type = TYPE_REPLY;
    message->ID = ID;
    message->op_code = NULL_OP_CODE;
    message->payload_len = 0u;
    message->payload = NULL;
}

//builds connect call, ID is provided from outside
//right now the payload is empty
void build_connect_call(struct MSG *retval, uint32_t ID){
    destroymsg(*retval);
    inner_build_template_call(retval, ID);
    retval->op_code = OP_CODE_CONNECT_CALL;
}

//builds reply on successful connection
//op_code is used as a STATUS in the reply message
void build_success_reply(struct MSG *retval, uint32_t ID){
    destroymsg(*retval);
    inner_build_template_reply(retval, ID);
    retval->op_code = STATUS_SUCCESS;
}


//builds reply on FAILED connection
//op_code is used as a STATUS in the reply message
//the reason why connection failed will be stated in status and payload contains more information
void build_fail_reply(struct MSG *retval, uint32_t ID, uint32_t reason, uint32_t length, unsigned char *payload){
    destroymsg(*retval);
    inner_build_template_reply(retval, ID);
    retval->op_code = reason;
    retval->payload_len = length;
    if(length == 0)
        retval->payload = NULL;
    else{
        retval->payload = (unsigned char*) calloc(length, 1);
        strncpy((char*)retval->payload, (char *)payload, length);
    }
}

//builds call to close connection
void build_quit_call(struct MSG *retval, uint32_t ID){
    destroymsg(*retval);
    inner_build_template_call(retval, ID);
    retval->op_code = OP_CODE_CLOSE_CONNECTION;
}

//ID for ping=pong is always 0, it is reserved
//is used to test if the client is still present
void build_ping_call(struct MSG *retval){
    destroymsg(*retval);
    inner_build_template_call(retval, 0);
    retval->op_code = OP_CODE_PING;
}
//is used to repsond to a ping call
void build_pong_call(struct MSG *retval){
    destroymsg(*retval);
    inner_build_template_reply(retval, 0);
    retval->op_code = STATUS_PONG;
}
