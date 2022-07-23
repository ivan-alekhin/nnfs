#include <stdio.h>
#include <string.h>

//main functionality of libnnfs_proto can be unit_tested
#include "libnnfs_proto.h"
void utest_proto();

//libnnfs_msg_builder only assigns values to MSG fields, it cant go wrong(only build_fail_reply may have problems as it operates on strings) 

//libnnfs_socket was tested "in the field" by creating client and a server and making them communicate
//the test for the functions will be the same as making a server and a client, so there is no reason to do it
//nnfs_client and NNFS_server are tested in conjuction with each other 

//nnfs_constants are just constants, they just need to have unique names and differnet values for the same prefix

//all functionality of libnnfs_primitive_matcher can be unit_tested
#include "libnnfs_primitive_matcher.h"
void utest_primitive_matcher();



int main(){
    printf("Testing libnnfs_proto.h functions:\n");
    utest_proto();
    printf("\n");

    printf("Testing libnnfs_priitive_matcher.h functions:\n");
    utest_primitive_matcher();
    printf("\n");

}

void utest_primitive_matcher(){
    printf("Trying to find ip and port in a string:\n");
    const char *contains_ip_port = "sads234.23.16.7:25555safsaf";
    char *ip_ptr = NULL, *port_ptr = NULL;
    match_IPaddr(contains_ip_port, &ip_ptr);
    if(ip_ptr != NULL)
        printf("There was IP-like structure in \"%s\"\nIt is \"%s\"\n", contains_ip_port, ip_ptr);
    else
        printf("There was no Ip-like structure in \"%s\"\n", contains_ip_port);
    match_PORTnumber(contains_ip_port, &port_ptr);
    if(port_ptr != NULL)
        printf("There was port-like structure in \"%s\"\nIt is \"%s\"\n", contains_ip_port, port_ptr);
    else
        printf("There was no port-like structure in \"%s\"\n", contains_ip_port);
    printf("match_number is a rip-off of match_PORTnumber\n");
}

#define PAYLOAD_STRING "Hello world!"
void utest_proto(){
    printf("----1)Encoding Message with no payload and then decoding it:----\n");
    struct MSG message;
    message.header.ID = 13u;
    message.header.op_code = 14u;
    message.header.payload_len = 0u;
    message.header.type = 15u;
    message.payload = NULL;
    printf("ID = %d\nop_code = %d\npayload_len = %d\ntype = %d\n", message.header.ID, message.header.op_code, message.header.payload_len, message.header.type);
    struct ENCODED_MESSAGE encmes;
    init_encmes(&encmes);
    encode(&message, &encmes);
    message.header.ID = 250u;
    message.header.op_code = 250u;
    message.header.payload_len = 0u;
    message.header.type = 250u;
    message.payload = NULL;    
    decode_header(&encmes, &message);
    printf("message was encoded and decoded:\n");
    printf("ID = %d\nop_code = %d\npayload_len = %d\ntype = %d\n", message.header.ID, message.header.op_code, message.header.payload_len, message.header.type);

    printf("----2)Encoding message with a payload and then decoding it:----\n");
    char * ptr = PAYLOAD_STRING;
    message.header.payload_len = sizeof(PAYLOAD_STRING) - 1;
    message.payload = calloc(message.header.payload_len, 1);
    strncpy((char *) message.payload,ptr, message.header.payload_len);
    printf("ID = %d\nop_code = %d\npayload_len = %d\ntype = %d\n", message.header.ID, message.header.op_code, message.header.payload_len, message.header.type);
    printf("payload is: \"");
    for(int i = 0; i < message.header.payload_len; i++){
        printf("%c", message.payload[i]);
    }
    printf("\"\n");
    encode(&message, &encmes);
    destroy_msg(&message);
    message.header.ID = 250u;
    message.header.op_code = 250u;
    message.header.payload_len = 0u;
    message.header.type = 250u;
    message.payload = NULL;
    decode_header(&encmes, &message);
    struct ENCODED_MESSAGE encmes_payload;
    encmes_payload.length = encmes.length - MSG_HEADER_SIZE;
    encmes_payload.mes = encmes.mes + MSG_HEADER_SIZE;
    decode_payload(&encmes_payload, &message);
    printf("message was encoded and decoded:\n");
    printf("ID = %d\nop_code = %d\npayload_len = %d\ntype = %d\n", message.header.ID, message.header.op_code, message.header.payload_len, message.header.type);
    printf("payload is: \"");
    for(int i = 0; i < message.header.payload_len; i++){
        printf("%c", message.payload[i]);
    }
    printf("\"\n");
    printf("The maximum size of the payload is %d\n", NNFS_MSG_MAX_LENGTH);
    printf("It is an artificial boundary made by checking if(payload_len < %d), so the segmentation of text will be added later if needed\n", NNFS_MSG_MAX_LENGTH);
}

