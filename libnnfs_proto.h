#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define MSG_HEADER_SIZE 16u
#define BYTES_IN_UINT32 sizeof(uint32_t) 
#define BITS_IN_BYTE 8u
#define TYPE_CALL 0u
#define TYPE_REPLY 1u


//ID used to send a reply to a specific call message by a server
//type distinguishes call from a reply
//op_code also serves as a status field in a reply message
struct MSG{
    uint32_t ID;
    uint32_t type;
    uint32_t op_code;
    uint32_t payload_len;
    unsigned char* payload;
};


//converts to XDR standard and expects at least for 4 bytes to be available in write_to parameter
void convert_uint32_to_XDR(uint32_t integer, unsigned char* write_to){
    for(int i = 0; i<BYTES_IN_UINT32; i++){
        write_to[i] = (unsigned char) (integer >> (BYTES_IN_UINT32 * BITS_IN_BYTE - (i+1) * BITS_IN_BYTE));
    }
}

//should work probably
uint32_t convert_XDR_to_uint32(unsigned char* read_from){
    uint32_t res = 0u;
    for(int i = 0; i<BYTES_IN_UINT32; i++){
        res = (res << BITS_IN_BYTE) + read_from[i];
    }
    return res;
}

//to make reading of the code easier
struct encoded_message{
    unsigned char *mes;
    uint32_t length;
};

//converts from C struct message to the XDR format that can be send by a socket
struct encoded_message encode(struct MSG toencode){
    struct encoded_message encmes;
    encmes.mes = (unsigned char*) calloc(1, MSG_HEADER_SIZE + toencode.payload_len);
    encmes.length = MSG_HEADER_SIZE + toencode.payload_len;

    //probably temporary
    convert_uint32_to_XDR(toencode.ID, encmes.mes);
    convert_uint32_to_XDR(toencode.type, encmes.mes + BYTES_IN_UINT32);
    convert_uint32_to_XDR(toencode.op_code, encmes.mes + 2* BYTES_IN_UINT32);
    convert_uint32_to_XDR(toencode.payload_len, encmes.mes + 3* BYTES_IN_UINT32);

    unsigned char* src_ptr = toencode.payload;
    unsigned char* dest_ptr = encmes.mes + 4*BYTES_IN_UINT32;
    for(int i = 0; i < toencode.payload_len; i++, src_ptr++,dest_ptr++){
        *dest_ptr = *src_ptr;
    }
    return encmes;
}

//TODO: debug
struct MSG decode(struct encoded_message encmes){
    struct MSG retval;
    retval.ID = convert_XDR_to_uint32(encmes.mes);
    retval.type = convert_XDR_to_uint32(encmes.mes + BYTES_IN_UINT32);
    retval.op_code = convert_XDR_to_uint32(encmes.mes + 2* BYTES_IN_UINT32);
    retval.payload_len = convert_XDR_to_uint32(encmes.mes + 3*BYTES_IN_UINT32);

    if(retval.payload_len == 0) 
        retval.payload = NULL;
    else{
        unsigned char* src_ptr = encmes.mes + 4*BYTES_IN_UINT32;
        retval.payload = (unsigned char*) calloc(retval.payload_len, 1);
        unsigned char *dest_ptr = retval.payload;
        for(int i = 0; i < retval.payload_len; i++, dest_ptr++, src_ptr++){
            *dest_ptr = *src_ptr;
        }
    }
    return retval;
}


//uniform way to destroy structs introduced in this lib
void destroy(struct MSG message){
    if(message.payload_len != 0){
        free(message.payload);
    }
    message.payload = NULL;
}

void destroy(struct encoded_message encmes){
    if(encmes.mes != NULL)
        free(encmes.mes);
    encmes.mes = NULL;
}
