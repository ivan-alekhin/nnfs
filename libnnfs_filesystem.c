#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "libnnfs_filesystem.h"

//considered to be root of the filesystem
char *hosting_directory = NULL;

#define LISTING_DIRECTORY_FORMAT "DIR: %s\n"
#define LISTING_FILE_FORMAT "FIL: %s\n"
#define LISTING_FORMAT_SIZE (sizeof(LISTING_DIRECTORY_FORMAT) -2)//2 is %s as they do not go into count
#define PATH_HOME "~/"
#define PATH_PARENT_DIRECTORY "/.."

int create_new_dir_descriptor(struct dir_descriptor *descr){
    descr->curdir_path = NULL;
    descr->current_entity = NULL;
    int retval = DIRECTORY_SUCCESS;
    if(hosting_directory == NULL){
        printf("ERROR: hosting_directory wasnt set\n");
        return DIRECTORY_FAILED_TO_OPEN;
    }

    descr->directory = opendir(hosting_directory);
    if(descr->directory == NULL){
        printf("ERROR: couldnt open hosting directory\n");
        return DIRECTORY_FAILED_TO_OPEN;
    }
    return DIRECTORY_SUCCESS;
}

int close_descriptor(struct dir_descriptor *dir){
    free(dir->curdir_path);
    return closedir(dir->directory);
}

int set_hosting_directory(const char *path){
    free(hosting_directory);
    hosting_directory = calloc(sizeof(char), strlen(path));
    strncpy(hosting_directory, path, strlen(path));
    return DIRECTORY_SUCCESS;
}

int list_directory(struct dir_descriptor *descr){
    if(descr->directory == NULL){
        printf("ERROR: the dir_descriptor wasnt created\n");
        return DIRECTORY_FAILED_TO_OPEN;
    }
    

    descr->current_entity = readdir(descr->directory);
    while(descr->current_entity != NULL){
        if(descr->current_entity->d_type == DT_DIR)
            if(strcmp(descr->current_entity->d_name, ".") != 0 && strcmp(descr->current_entity->d_name, "..") != 0)
                printf("DIR: %s\n", descr->current_entity->d_name);

        if(descr->current_entity->d_type == DT_REG)
            printf("FIL: %s\n", descr->current_entity->d_name);

        descr->current_entity = readdir(descr->directory);
    }

    return DIRECTORY_SUCCESS;
}

int slist_directory(struct dir_descriptor *descr, char *str, uint32_t str_size){
    if(descr->directory == NULL){
        printf("ERROR: the dir_descriptor wasnt created\n");
        return DIRECTORY_FAILED_TO_OPEN;
    }
    uint32_t current_fillament = 0u;
    uint32_t space_needed = 0u;

    if(descr->current_entity == NULL)
        descr->current_entity = readdir(descr->directory);
    
    bool enough_space_in_str = true;
    str_size-=1u;

    while(enough_space_in_str && descr->current_entity != NULL){
        //printf("\nSTATUS: currently \nspace_needed = %d\nenough_space_in_str = %d\ncurrent_fillament = %d\nfile_name = %s\n\n", space_needed, enough_space_in_str, current_fillament, descr->current_entity->d_name);
        if(strcmp(descr->current_entity->d_name, ".") != 0 && strcmp(descr->current_entity->d_name, "..") != 0){
            space_needed = LISTING_FORMAT_SIZE + strlen(descr->current_entity->d_name);

            if(space_needed + current_fillament < str_size){
                if(descr->current_entity->d_type == DT_REG)
                    snprintf(str + current_fillament,space_needed, LISTING_FILE_FORMAT, descr->current_entity->d_name);

                if(descr->current_entity->d_type == DT_DIR)
                    snprintf(str + current_fillament,space_needed, LISTING_DIRECTORY_FORMAT, descr->current_entity->d_name);

                current_fillament += LISTING_FORMAT_SIZE + strlen(descr->current_entity->d_name) - 1;
                
            }
            else enough_space_in_str = false;
        }
        if(enough_space_in_str)
            descr->current_entity = readdir(descr->directory);
        
    }
    str[current_fillament] = '\0';
    if(enough_space_in_str)
        return DIRECTORY_SUCCESS;
    else
        return DIRECTORY_NOT_EVERYTHING_LISTED;
}

//checks if its an absolute path
bool is_absolute_path(const char *path){
    if(strncmp(path, PATH_HOME, sizeof(PATH_HOME) - 1) == 0)
        return true;
    return false;
}

//counts .. in array
int count_parentcalls(const char *beg_of_path){
    if(beg_of_path == NULL)
        return 0;
    int counter = 0;
    char *pch=strstr(beg_of_path,"..");
    while (pch!=NULL) {
        counter++;
        pch=strstr(pch+1,"..");
    }
    return counter;
}

//counts directories in the array
int count_dir_in_path(const char *beg_of_path){
    if(beg_of_path == NULL)
        return 0;
    int counter = 1;
    char *pch=strchr(beg_of_path,'/');
    while (pch!=NULL) {
        counter++;
        pch=strchr(pch+1,'/');
    }
    return counter;     
}

//checks for dots after words, they so that its not possible to write example./../../ and escape hosting dir 
bool dots_are_in_right_placces(const char * path){
    const char *pch=strchr(path,'.');
    if(pch == path)
        pch = strchr(pch+1,'.');
    while (pch!=NULL){
        if(pch[-1] != '.' && pch[-1] != '/'){
            return false;
        }
        pch=strchr(pch+1,'.');
    }
    return true;
}
//returns true if all parentcalls are preceeded by only parentcalls
bool parentcall_preceeding(const char * path){
    //bool parentcall_prec = (strncmp(path, "../", sizeof("../") - 1) == 0);
    const char *pch=strchr(path,'/');
    while (pch!=NULL){
        if(pch - path + 1 < strlen(path)){
            if(pch[1] == '.' && pch[-1] != '.'){
                return false;
            }
        }
        pch=strchr(pch+1,'/');
    }
    return dots_are_in_right_placces(path);
}

//checks for safety in the path(to not escape hosting dir)
bool is_safe_path(const char *path, const struct dir_descriptor* descr){
    //path cant start or end with /
    if(strchr(path, '/') == path || path[strlen(path) - 1] == '/')
        return false;

    //it's not allowed to reference /. in any path (can cause memory overflow)
    if(strncmp(path, "./", sizeof("./") -1) == 0)
        return false;
    if(strstr(path, "/./") != NULL)
        return false;
    if(strcmp(path + strlen(path) - sizeof("/."), "/.") == 0)
        return false;

    //checking if its an absolute path it cant have parentcalls /..
    //if it doesnt have parentcalls it's considered safe
    if(is_absolute_path(path))
        if(strstr(path, PATH_HOME) != NULL){
            return false;
        }
        else 
            return true;
    //only relative path are here
    //all parentcalls must be in the beginning  /../.. is ok, but /whatever/.. is not
    if(!parentcall_preceeding(path))
        return false;
    
    //the amount of parentcalls must be lower than the amount of directories opened from the home directory
    if(count_parentcalls(path) > count_dir_in_path(descr->curdir_path))
        return false;

    return true;
}

//finds last dir to be saved before ..
int find_last_dir(const char *beg_of_path, int number_of_slashes){
    const char * pch = beg_of_path;
    while(number_of_slashes > 0){
        pch = strchr(pch + 1, '/');
        number_of_slashes--;
    }
    return (int) (pch - beg_of_path);
}

//gets rid of .. to optimise memory
void build_whole_path(const struct dir_descriptor *descr, const char *path, char **new_path){
    if(is_safe_path(path, descr)){

        if(is_absolute_path(path)){
            *new_path = calloc(1, strlen(path));
            strcpy(*new_path, path);
            return;
        }

        int parentcalls_number = count_parentcalls(path);
        int cur_dir_left_dir = count_dir_in_path(descr->curdir_path) - parentcalls_number;
        printf("parent calls: %d\n", parentcalls_number);
        printf("cur_dir = %d\n", cur_dir_left_dir);
        if(cur_dir_left_dir < 0)
            return;

        if(parentcalls_number == 0){
            if(descr->curdir_path != NULL)
                cur_dir_left_dir = strlen(descr->curdir_path);
            else
                cur_dir_left_dir = parentcalls_number;
        }
        else
            cur_dir_left_dir = find_last_dir(descr->curdir_path, cur_dir_left_dir);

        free(*new_path);
        //the first parentcall is without a slash
        const char * after_the_parentcalls = path;
        if(parentcalls_number != 0){
            after_the_parentcalls+= parentcalls_number * sizeof(PATH_PARENT_DIRECTORY) - 2;
        }
        
        *new_path = calloc(1, cur_dir_left_dir + strlen(after_the_parentcalls) + 1);
        if(cur_dir_left_dir == 0){
            strcat(*new_path, after_the_parentcalls);
        }
        else{
            strncat(*new_path, descr->curdir_path, cur_dir_left_dir);
            strcat(*new_path, "/");
            strcat(*new_path, after_the_parentcalls);
        }
    }
    else *new_path = NULL;
}

int change_directory(struct dir_descriptor *descr, const char *dir){
    if(hosting_directory == NULL){
        printf("ERROR: hosting_directory wasnt set\n");
        return DIRECTORY_FAILED_TO_OPEN;
    }

    char *new_path = NULL;
    build_whole_path(descr, dir, &new_path);
    if(new_path == NULL)
        return DIRECTORY_PERMISSION_DENIED;
    
    printf("STATUS: new path equals = \"%s\"\n",new_path);
    char *absolute_path = calloc(1, strlen(hosting_directory) + strlen(new_path) + 1);
    strcat(absolute_path, hosting_directory);
    strcat(absolute_path, "/");
    strcat(absolute_path, new_path);

    DIR *new_dir = opendir(absolute_path);
    printf("STATUS: opening at \"%s\"\n", absolute_path);
    free(absolute_path);
    if(new_dir == NULL){
        printf("ERROR: couldnt open directory\n");
        free(new_path);
        return DIRECTORY_FAILED_TO_OPEN;
    }
    else{
        printf("SUCCESS: opened directory\n");
        closedir(descr->directory);
        descr->directory = new_dir;
        new_dir = NULL;
        free(descr->curdir_path);
        if(strlen(new_path) != 0){
            descr->curdir_path = calloc(1, strlen(new_path));
            strcpy(descr->curdir_path, new_path);
        }
        else
            descr->curdir_path = NULL;
        free(new_path);
        return DIRECTORY_SUCCESS;
    }
}

void rewind_directory(struct dir_descriptor *dir){
    rewinddir(dir->directory);
}

bool is_safe_filename(const char *filename){
    if(strchr(filename, '/') != NULL){
        printf("ERROR: files can be opened only in the current directory\n");
        return false;
    }
    return true;
}

int write_to_file_internal(const char *filename, const char *buffer){
    FILE *file_descr = fopen(filename, "a");
    if(file_descr == NULL){
        printf("ERROR: failed to create a file : \"%s\"\n", filename);
        return DIRECTORY_FAIL_TO_CREATE_FILE;
    }

    fprintf(file_descr, "%s\n", buffer);
    fclose(file_descr);
    return DIRECTORY_SUCCESS;
}

int write_to_file(const struct dir_descriptor *descr, const char *filename,  const char *buffer){
    char *file_path = NULL;
    if(!is_safe_filename(filename)){
        printf("ERROR: file can only be opened in the current directory\n");
        return DIRECTORY_FILE_OUTSIDE_OF_CURDIR;
    }

    if(descr->curdir_path != NULL)
        file_path = calloc(1, strlen(filename) + strlen(hosting_directory) + strlen(descr->curdir_path) + 3);
    else
        file_path = calloc(1, strlen(filename) + strlen(hosting_directory) + 3);
    strcat(file_path, hosting_directory);
    strcat(file_path, "/");
    if(descr->curdir_path != NULL){
        strcat(file_path, descr->curdir_path);
        strcat(file_path, "/");
    }
    strcat(file_path, filename);

    int result = write_to_file_internal(file_path, buffer);
    free(file_path);
    return result;
}

int read_from_file_internal(const char *filename, char *buffer, uint32_t size_of_buffer, uint32_t offset){
    FILE *file_descr = fopen(filename, "r");
    if(file_descr == NULL){
        printf("ERROR: failed to read from file: \"%s\"\n", filename);
        return  DIRECTORY_FAILED_TO_OPEN;
    }
    //saving last character for NULL char
    size_of_buffer--;
    if(fseek(file_descr, offset, SEEK_SET) != 0){
        printf("ERROR: couldnt get to the position %d\n", offset);
        return DIRECTORY_FAILED_TO_GET_TO_OFFSET;
    }

    char ch = fgetc(file_descr);
    uint32_t index = 0;
    while(size_of_buffer > index && ch != EOF){
        buffer[index] = ch;
        index++;
        ch = fgetc(file_descr);
    }
    buffer[index] = 0;
    fclose(file_descr);

    if(ch != EOF)
        return offset + index;
    else
        return DIRECTORY_SUCCESS;
}

int read_from_file(const struct dir_descriptor *descr,const char *filename,  char *buffer, uint32_t size_of_buffer, uint32_t offset){
    char *file_path = NULL;
    if(!is_safe_filename(filename)){
        printf("ERROR: file can only be opened in the current directory\n");
        return DIRECTORY_FILE_OUTSIDE_OF_CURDIR;
    }
    if(descr->curdir_path != NULL)
        file_path = calloc(1, strlen(filename) + strlen(hosting_directory) + strlen(descr->curdir_path) + 3);
    else
        file_path = calloc(1, strlen(filename) + strlen(hosting_directory) + 3);
    strcat(file_path, hosting_directory);
    strcat(file_path, "/");
    if(descr->curdir_path != NULL){
        strcat(file_path, descr->curdir_path);
        strcat(file_path, "/");
    }
    strcat(file_path, filename);

    int result = read_from_file_internal(file_path, buffer, size_of_buffer, offset);
    free(file_path);

    return result;
}