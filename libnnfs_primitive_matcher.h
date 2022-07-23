#pragma once

//matching ipv4 address and port from a command
void match_IPaddr(const char *,char **);
void match_PORTnumber(const char *, char **);

//gets a number from a string
int match_client_number(const char *);


//matchers for distinguishing user input commands
//gets the type of command
int type_of_command(const char *);