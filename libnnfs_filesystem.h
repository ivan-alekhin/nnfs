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
//asks to provide more space to list the remaining items, it tried to push everything it could into the given buffer
//but couldnt push everything and asks for more 
#define DIRECTORY_NOT_EVERYTHING_LISTED 1
#define DIRECTORY_PERMISSION_DENIED -2
#define DIRECTORY_FAIL_TO_CREATE_FILE -3
#define DIRECTORY_FILE_OUTSIDE_OF_CURDIR -4
#define DIRECTORY_FAILED_TO_GET_TO_OFFSET -5

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
//writes to file in the same directory
int write_to_file(const struct dir_descriptor *descr, const char *filename,  const char *buffer);
//read from file
//returns offset where it stopped reading if successfull
//returns 0 if everything from file was read
//returns negative values if it failed
int read_from_file(const struct dir_descriptor *descr, const char *filename,  char *buffer, uint32_t size_of_buffer, uint32_t offset);