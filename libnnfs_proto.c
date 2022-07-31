#include "libnnfs_proto.h"
#include "nnfs_constants.h"

#include <stddef.h>
#include <assert.h>
#include <stdio.h>

struct MSG_HEADER;
struct MSG;

#define BYTES_IN_UINT32 sizeof(uint32_t) 
#define BITS_IN_BYTE 8u

//converts to XDR standard and expects at least for 4 bytes to be available in write_to parameter
void convert_uint32_to_XDR(uint32_t integer, unsigned char *write_to){
    for(int i = 0; i<BYTES_IN_UINT32; i++){
        write_to[i] = (unsigned char) (integer >> (BYTES_IN_UINT32 * BITS_IN_BYTE - (i+1) * BITS_IN_BYTE));
    }
}

//xdr representation to uint32
uint32_t convert_XDR_to_uint32(unsigned char *read_from){
    uint32_t res = 0u;
    for(int i = 0; i<BYTES_IN_UINT32; i++){
        res = (res << BITS_IN_BYTE) + read_from[i];
    }
    return res;
}

void destroy_msg(struct MSG *message){
    free(message->payload);
    message->header.payload_len = 0u;
    message->payload = NULL;
}

void destroy_encmes(struct ENCODED_MESSAGE *encmes){
    free(encmes->mes);
    encmes->mes = NULL;
    encmes->length = 0u;
}

void init_encmes(struct ENCODED_MESSAGE *encmes){
    encmes->length = 0u;
    encmes->mes = NULL;
}

void init_msg(struct MSG *message){
    message->payload = NULL;
    message->header.payload_len = 0u;
}

void decode_header(const struct ENCODED_MESSAGE *encmes, struct MSG *message){
    if(message->header.payload_len != 0u){
        destroy_msg(message);
        message->payload = NULL;
    }
    message->header.payload_len = 0u;
    message->header.ID = convert_XDR_to_uint32(encmes->mes);
    message->header.type = convert_XDR_to_uint32(encmes->mes + BYTES_IN_UINT32);
    message->header.op_code = convert_XDR_to_uint32(encmes->mes + 2* BYTES_IN_UINT32);
    message->header.payload_len = convert_XDR_to_uint32(encmes->mes + 3*BYTES_IN_UINT32);
    message->header.is_last = encmes->mes[4 * BYTES_IN_UINT32];
    message->header.number_in_sequence = encmes->mes[4 * BYTES_IN_UINT32 + 1];
    if(ENABLE_LOGGING != 0)
        printf("SUCCESS: header decoded\n");
}

void encode_header(const struct MSG *message, struct ENCODED_MESSAGE *encmes){
    convert_uint32_to_XDR(message->header.ID, encmes->mes);
    convert_uint32_to_XDR(message->header.type, encmes->mes + BYTES_IN_UINT32);
    convert_uint32_to_XDR(message->header.op_code, encmes->mes + 2* BYTES_IN_UINT32);
    convert_uint32_to_XDR(message->header.payload_len, encmes->mes + 3* BYTES_IN_UINT32);
    encmes->mes[4 * BYTES_IN_UINT32] = message->header.is_last;
    encmes->mes[4 * BYTES_IN_UINT32 + 1] = message->header.number_in_sequence;
}

void encode(const struct MSG *message, struct ENCODED_MESSAGE *encmes){
    assert(message->header.payload_len <= NNFS_MSG_MAX_LENGTH);
    if(encmes->length != 0)
        destroy_encmes(encmes);

    encmes->length = MSG_HEADER_SIZE + message->header.payload_len;
    encmes->mes = calloc(1, encmes->length);
    if(encmes->mes == NULL){
        printf("ERROR: calloc in encode returned NULL\n");
        exit(1);
    }
    encode_header(message, encmes);
    unsigned char *dest = encmes->mes + MSG_HEADER_SIZE;
    unsigned char *src = message->payload;
    for(int i = 0; i < message->header.payload_len; i++, src++ ,dest++){
        *dest = *src;
    }
    if(ENABLE_LOGGING != 0)
        printf("SUCCESS: msg encoded\n"); 
}

void decode_payload(const struct ENCODED_MESSAGE *encmes, struct MSG *message){
    assert(message->header.payload_len <= NNFS_MSG_MAX_LENGTH);
    if(message->header.payload_len == 0u){
        message->payload = NULL;
        return;
    }

    message->payload = calloc(1, message->header.payload_len);
    if(message->payload == NULL){
        printf("ERROR: calloc in decode_payload returned NULL\n");
        exit(1);
    }
    unsigned char *src = encmes->mes;
    unsigned char *dest = message->payload;
    for(int i = 0; i < message->header.payload_len; i++, dest++, src++){
        *dest = *src;
    }
    if(ENABLE_LOGGING != 0)
        printf("SUCCESS: payload decoded\n");
}