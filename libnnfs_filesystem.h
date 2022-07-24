#pragma once
#include <dirent.h>
#include <stddef.h>
#include <stdint.h>

struct dir_descriptor{
    DIR *directory;
    //to keep listing instead of fully refreshing each time
    struct dirent* current_entity;
    //absolute path to file from the root - hosting directory
    char *curdir_path;
};

//retvalues
#define DIRECTORY_SUCCESS 0
#define DIRECTORY_FAILED_TO_OPEN -1
#define DIRECTORY_NOT_EVERYTHING_LISTED 1
#define DIRECTORY_PERMISSION_DENIED 2

//creating file descriptor
int create_new_dir_descriptor(struct dir_descriptor *current_dir);
//list directory into array, will keep on giving new data if at least one at a time can fit into the given array
int slist_directory(struct dir_descriptor *current_dir, char *array_to_write_to, uint32_t size);
//prints directory in stdin
int list_directory(struct dir_descriptor *current_dir);
//changes directory - safe (it took all evening to make it safe)
int change_directory(struct dir_descriptor *current_dir, const char *directory);
//closing descriptor with freeing memory
int close_descriptor(struct dir_descriptor *dir);
//set hosting directory with no way to get outside of it
int set_hosting_directory(const char *path);
//rewinds directory
void rewind_directory(struct dir_descriptor *dir);