#include "libnnfs_primitive_matcher.h"
#include "nnfs_constants.h"


#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//I know that they look ridiculous, for some reason those scary // are needed before any regex special character
const char *IPregex = "\\([0-9]\\{1,3\\}\\.\\)\\{3\\}[0-9]\\{1,3\\}";
const char *PORTregex = ":[0-9]\\{1,5\\}";
const char *NUMBERregex = "[0-9]\\+";

//formats for commands
const char *connect_format = "connect";
const char *ping_format = "ping";
const char *help_format = "help";
const char *quit_format = "quit";
const char *bind_format = "bind";
const char *listen_and_accept_format = "start";

void match_IPaddr(const char *str, char **dest){
    free(*dest);
    regex_t ip;
    if(regcomp(&ip, IPregex, 0) != 0){
        printf("ERROR: couldnt compile regex in match_IPaddr\n");
    }

    regmatch_t IPmatch;
    int status = regexec(&ip, str, 1, &IPmatch, 0);
    if(status == 0){
        *dest = calloc(1, IPmatch.rm_eo - IPmatch.rm_so + 1);
        //logger
        strncpy(*dest,str + IPmatch.rm_so, IPmatch.rm_eo - IPmatch.rm_so);
        printf("SUCCESS: matched ip = %s\n", *dest);
    }
    else if(status == REG_NOMATCH){
        printf("WARNING: regexec in match_IPaddr could find ip\n");
        *dest = NULL;
    }
    else{
        printf("ERROR: regexec in match_IPaddr failed\n");
        *dest = NULL;
    }

}

void match_PORTnumber(const char *str, char **dest){
    free(*dest);
    regex_t port;
    if(regcomp(&port, PORTregex, 0) != 0){
        printf("ERROR: couldnt compile regex in match_PORTnumber\n");
    }
    regmatch_t PORTmatch;
    int status = regexec(&port, str, 1, &PORTmatch, 0);
    
    if(status == 0){
        
        *dest = calloc(1, PORTmatch.rm_eo - PORTmatch.rm_so + 1);
        if(*dest == NULL){
            printf("ERROR: calloc returned NULL in match_PORTnumber\n");
        }
        strncpy(*dest, str + PORTmatch.rm_so + 1, PORTmatch.rm_eo - PORTmatch.rm_so - 1);
        printf("SUCCESS: matched port = %s\n", *dest);
    }
    else if(status == REG_NOMATCH){
        printf("WARNING: regexec in match_PORTnumber couldnt find port\n");
        *dest = NULL;
    }
    else{
        printf("ERROR: regexec in match_PORTnumber failed\n");
        *dest = NULL;
    }
}

int type_of_command(const char *str){
    if(strstr(str, connect_format) != NULL)     return OP_CODE_CONNECT_CALL;
    if(strstr(str, ping_format) != NULL)    return OP_CODE_PING;
    if(strstr(str, quit_format) != NULL)    return OP_CODE_CLOSE_CONNECTION;
    if(strstr(str, bind_format) != NULL)    return SERVER_BIND;
    if(strstr(str, listen_and_accept_format) != NULL)   return SERVER_LISTEN_AND_ACCEPT;
    return NULL_OP_CODE;
}

int match_client_number(const char *str){
    regex_t number;
    if(regcomp(&number, NUMBERregex, 0) != 0){
        printf("ERROR: couldnt compile regex in match_client_number\n");
    }
    regmatch_t NUMBERmatch;
    int status = regexec(&number, str, 1, &NUMBERmatch, 0);
    //printf("NUMBER matched: %d\n", status);
    if(status == 0){
        //printf("from %d to %d\n",PORTmatch.rm_so, PORTmatch.rm_eo);
        char *dest = calloc(1, NUMBERmatch.rm_eo - NUMBERmatch.rm_so + 1);
        //logger
        strncpy(dest, str + NUMBERmatch.rm_so, NUMBERmatch.rm_eo - NUMBERmatch.rm_so);
        int value = atoi(dest);
        free(dest);
        printf("SUCCESS: successfully matched number = %d\n", value);
        return value;
    }
    else if(status == REG_NOMATCH){
        printf("WARNING: regexec in match_client_number couldnt find any number\n");
        return -1;
    }
    else{
        printf("ERROR: regexec in match_client_number failed\n");
        return -1;
    }
}