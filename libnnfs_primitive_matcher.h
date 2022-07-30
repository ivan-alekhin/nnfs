#pragma once
#include <stdint.h>

//matching ipv4 address and port from a command
void match_IPaddr(const char *,char **);
void match_PORTnumber(const char *, char **);

//gets a number from a string
int match_client_number(const char *);


//matchers for distinguishing user input commands
//gets the type of command
int type_of_command(const char *);

struct read_info{
    uint32_t    offset,
                number_of_characters,
                read_mode;
    char * file_path;
};

int get_read_info_from_call(const char *payload, struct read_info *info);

struct write_info{
    char *file_path;
    char *buffer; 
};

int get_write_info_from_call(const char *payload, const uint32_t length, struct write_info *info);