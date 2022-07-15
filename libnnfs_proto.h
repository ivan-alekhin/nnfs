#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

//ID used to send a reply to a specific call message by a server
//type distinguishes call from a reply
//op_code also serves as a status field in a reply message
struct MSG_HEADER{
    uint32_t ID;
    uint32_t type;
    uint32_t op_code;
    uint32_t payload_len;
};
#define MSG_HEADER_SIZE sizeof(struct MSG_HEADER)

struct MSG{
    struct MSG_HEADER header;
    unsigned char*payload;
};

//to make reading of the code easier
struct ENCODED_MESSAGE{
    unsigned char* mes;
    uint32_t length;
};
//uniform way to destroy and init structs introduced in this lib
void destroy_msg(struct MSG*);
void destroy_encmes(struct ENCODED_MESSAGE*);
void init_encmes(struct ENCODED_MESSAGE*);
void init_msg(struct MSG *);

//converts from C struct message to the XDR format that can be send by a socket
void encode(const struct MSG*, struct ENCODED_MESSAGE *);

//decodes from XDR format to C struct message
//should be called one after another:
//header is fixed size and contains the field that tells decode_payload how much data to expect
//also allows to recv know how much data there is in payload to receive
void decode_header(const struct ENCODED_MESSAGE *, struct MSG*);

//expects that at the start of the encoded_message is the encoded payload
void decode_payload(const struct ENCODED_MESSAGE *, struct MSG*);
