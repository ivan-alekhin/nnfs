#include <stdio.h>

#include "libnnfs_filesystem.h"
#define BUFFER_SIZE 256u

//testing place for the libnnfs_filesystem.c

const char * ptr = "../path/place";
char * CURDIR = "date/whatever";

int main(){
    struct dir_descriptor descr;
    set_hosting_directory("..");
    create_new_dir_descriptor(&descr);
    list_directory(&descr);
    printf("\ncould change dir %d\n", change_directory(&descr, "NNFS"));
    list_directory(&descr);
    change_directory(&descr, "..");
    list_directory(&descr);
    printf("STATUS: %d\n", change_directory(&descr, "../.."));
    rewind_directory(&descr);
    list_directory(&descr);
}