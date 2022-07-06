#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define AUTH_NONE 0
#define MSG_CALL_HEADER_SIZE 24u
#define MSG_REPLY_HEADER_SIZE 24u
#define BYTES_IN_UINT32 4u
#define BITS_IN_BYTE 8u
#define TYPE_CALL 0u
#define TYPE_REPLY 1u

//ID used to send a reply to a specific call message by the server
//message that contains the operation call and argument list in the payload section, that is represented as a byte array of variable length
//TODO: add auth system, right now everyone is anonymous
struct msg_call{
    //msg_type CALL goes here
    uint32_t ID;
    uint32_t    OP_CODE; //will be used by a level above to discriminate commands
    uint32_t    temp_USER_CRED; 
    uint32_t    temp_USER_VERF;
    uint32_t    PAYLOAD_LEN;
    
    char * PAYLOAD;
};

//ID used to discriminate message to reply to
//TODO: add auth system, right now everyone is anonymous
struct msg_reply{
    //msg_type REPLY goes here
    uint32_t    ID,
                temp_SERVER_VERF;
    
    //to distinguish if message was accepted for it is the same version and auth(not implemented yet) is correct
    //right now all calls are considered accepted
    enum enum_reply_stat {
         MSG_ACCEPTED = 0,
         MSG_DENIED   = 1
    };
    enum enum_reply_stat REPLY_STATUS;

    //to distinguish he state of the operation in the call
    enum enum_accept_stat{
        SUCCESS = 0, // operation successful 
        PROG_UNAVAIL = 1, //program wasnt exported (right now no use in plan)
        PROG_MISMATCH = 2, //program doesnt support the version(right now no use in plan)
        PROC_UNAVAIL = 3, //Unsupported procedure
        GARBAGE_ARGS = 4, //garbage arguments
        SYSTEM_ERR = 5 //other server-side errors
    };
    enum enum_accept_stat ACCEPT_STATUS;
    //for PROG_MISMATCH case sending the highest and lowest supported versions(right now no use in plan)
    //uint32_t low, high;


    //stating the reason for rejecting the reply
    enum enum_reject_stat{
        RPC_MISMATCH = 0,
        AUTH_ERROR = 1
    };
    enum enum_reject_stat REJECT_STATUS;

};


//converts to XDR standard and expects at least for 4 bytes to be available in write_to parameter
void convert_uint32_to_XDR(uint32_t integer, char* write_to){
    unsigned char * dest = (unsigned char * ) write_to;
    for(int i = 0; i<BYTES_IN_UINT32; i++){
        dest[i] = (unsigned char) (integer >> (BYTES_IN_UINT32 * BITS_IN_BYTE - (i+1) * BITS_IN_BYTE));
    }
}

//should work probably
uint32_t convert_XDR_to_uint32(char* read_from){
    uint32_t res = 0u;
    unsigned char* ptr = (unsigned char*) read_from;
    for(int i = 0; i<BYTES_IN_UINT32; i++){
        res = (res << BITS_IN_BYTE) + ptr[i];
    }
    return res;
}

struct encoded_message{
    char *mes;
};



struct encoded_message encode(struct msg_call call){
    struct encoded_message codedmsg;
    codedmsg.mes = (char *) calloc(MSG_CALL_HEADER_SIZE + call.PAYLOAD_LEN, 1);//to zero everything out
    
    convert_uint32_to_XDR(TYPE_CALL,codedmsg.mes);

    uint32_t *head = (uint32_t *) &call;
    for(int i = 0; i < (MSG_CALL_HEADER_SIZE /BYTES_IN_UINT32 - 1) ; i++){
        convert_uint32_to_XDR(*head, codedmsg.mes + (i + 1) * BYTES_IN_UINT32);
        head++;
    }

    char *mes_ptr = codedmsg.mes + MSG_CALL_HEADER_SIZE;
    char *payload_ptr = call.PAYLOAD;
    for(int i = 0; i < call.PAYLOAD_LEN; i++){
        *mes_ptr = *payload_ptr;
        mes_ptr++;
        payload_ptr++;
    }

    return codedmsg;
}
