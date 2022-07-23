#pragma once
//library's main purpose is to set meanings to values of MSG.header.op_code
//numbers are chosen on random, may change in the future


//types of messages
#define TYPE_REPLY 0u
#define TYPE_CALL 1u


//NULL_OP_CODE will be useful in troubleshooting, it represents no value
#define NULL_OP_CODE 256u 
#define OP_CODE_CONNECT_CALL 257u
#define OP_CODE_CLOSE_CONNECTION 258u
#define OP_CODE_PING 259u

//not implemented op_codes:
#define OP_CODE_LIST_DIRECTORY 259u
#define OP_CODE_CHANGE_DIRECTORY 260u
#define OP_CODE_WRITE_FROM_LOCAL 261u
#define OP_CODE_READ_FROM_REMOTE 262u


//op_code in TYPE_REPLY represents status
#define STATUS_SUCCESS 0u
//temporary uniform status fail, probably will be expanded to FAIL_<REASON>
#define STATUS_PONG 1u
#define STATUS_FAIL 2u
#define STATUS_FAIL_NO_SUCH_DIRECTORY 3u
#define STATUS_FAIL_BAD_OP_CODE 4u


//ping pong can be sent by both the client and the server so correctly managing IDs that allows server to respond to a specific 
//client message is problematic without it 
#define PING_PONG_ID 0u

//client leads the server: server responds to calls with the same ID that is ina the call message originally
#define STARTING_ID 1u

//commands that do not match op_codes
#define SERVER_BIND 513u
#define SERVER_LISTEN_AND_ACCEPT 514u
