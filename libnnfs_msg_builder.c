#include "libnnfs_msg_builder.h"
#include "nnfs_message_codes.h"
#include "libnnfs_proto.h"

#include <string.h>
#include <stddef.h>

//defines a basic template call
void inner_build_template_call(struct MSG *message, uint32_t ID){
    message->header.type = TYPE_CALL;
    message->header.ID = ID;
    message->header.op_code = NULL_OP_CODE;
    message->header.payload_len = 0u;
    message->payload = NULL;
}

//defines a basic template reply
void inner_build_template_reply(struct MSG *message, uint32_t ID){
    message->header.type = TYPE_REPLY;
    message->header.ID = ID;
    message->header.op_code = NULL_OP_CODE;
    message->header.payload_len = 0u;
    message->payload = NULL;
}


void build_connect_call(struct MSG *message, uint32_t ID){
    destroy_msg(message);
    inner_build_template_call(message, ID);
    message->header.op_code = OP_CODE_CONNECT_CALL;
}

void build_success_reply(struct MSG *retval, uint32_t ID){
    destroy_msg(retval);
    inner_build_template_reply(retval, ID);
    retval->header.op_code = STATUS_SUCCESS;
}

void build_fail_reply(struct MSG *retval, uint32_t ID, uint32_t reason, uint32_t length, unsigned char *payload){
    destroy_msg(retval);
    inner_build_template_reply(retval, ID);
    retval->header.op_code = reason;
    retval->header.payload_len = length;
    if(length == 0)
        retval->payload = NULL;
    else{
        retval->payload = (unsigned char*) calloc(length, 1);
        strncpy((char*)retval->payload, (char *)payload, length);
    }
}
void build_quit_call(struct MSG *retval, uint32_t ID){
    destroy_msg(retval);
    inner_build_template_call(retval, ID);
    retval->header.op_code = OP_CODE_CLOSE_CONNECTION;
}

void build_ping_call(struct MSG *retval){
    destroy_msg(retval);
    inner_build_template_call(retval, 0);
    retval->header.op_code = OP_CODE_PING;
}


void build_pong_reply(struct MSG *retval){
    destroy_msg(retval);
    inner_build_template_reply(retval, 0);
    retval->header.op_code = STATUS_PONG;
}