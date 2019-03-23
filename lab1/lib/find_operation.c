#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "find_operation.h"

size_t _array_size = 0;
size_t _next_index = 0;

int check_file_exists(const char * filename){
    FILE* file = fopen(filename, "r");
    if (file){
        fclose(file);
        return 1;
    }
    return 0;
}

int set_tab_size(size_t array_size){
    if(array_size < 1){
        fputs("Size of array could not be smaller than 1.\n", stderr);
        return 0;
    }
    _array_size = array_size;
    return 1;
}

void set_search_directory(char* const search_directory){
    _search_directory = search_directory;
}

void set_search_file_name(char* const search_file_name){
    _search_file_name = search_file_name;
}

int set_tmp_file_name(char* const tmp_file_name){
    if(check_file_exists(tmp_file_name)){
        fputs("File with the same name like tmp_file already exists.\n", stderr);
        return 0;
    }
    _tmp_file_name = tmp_file_name;
    return 1;
}

int init(){
    if(_array_size < 1){
        fputs("Before initialization array, you have to set array size.\n", stderr);
        return 0;
    }
    _operation_results = calloc(_array_size, sizeof(char*));
    return 1;
}

int find(){
    printf("start");
    char part1[] = "find ";
    char part2[] = " -name ";
    char part3[] = " 1>>./";

    size_t size1  = strlen(part1);
    size_t size2  = strlen(part2);
    size_t size3  = strlen(part3);
    size_t size_dir_name  = strlen(_search_directory);
    size_t size_file_name  = strlen(_search_file_name);
    size_t size_tmp_file_name  = strlen(_tmp_file_name);
    size_t command_size = size1 + size2 + size3 + size_dir_name + size_file_name + size_tmp_file_name;

    char *command = calloc(command_size + 1, sizeof(char*));
    if(!command){
        fputs("Memory alloc fails.\n", stderr);
        return 0;
    }

    memcpy(command, part1  , size1);
    memcpy(command + size1, _search_directory, size_dir_name);
    memcpy(command + size1 + size_dir_name, part2, size2);
    memcpy(command + size1 + size_dir_name + size2, _search_file_name, size_file_name);
    memcpy(command + size1 + size_dir_name + size2 + size_file_name, part3, size3);
    memcpy(command + size1 + size_dir_name + size2 + size_file_name + size3, _tmp_file_name, size_tmp_file_name);

    //set end of string
    command[command_size] = '\0';
    //execute command find and save output to tmp file
    int result = system(command);
    if(result != 0) return 0;

    free(command);

    return 1;
}

int load_content_from_tmp_file(){
    //sprawdzić czy plik o podanej nazwie już nie istnieje!!

    if(_next_index >= _array_size){
        remove(_tmp_file_name);
        printf("Not enough space in array to load content from file.\n");
        return -1;
    }

    //load tmp file content
    FILE *tmp_file;
    long file_size;
    char *file_content;

    tmp_file = fopen(_tmp_file_name, "rb");
    if(!tmp_file){
        remove(_tmp_file_name);
        fputs("Error while opening tmp_file.\n", stderr);
        return -1;
    }

    fseek(tmp_file , 0L, SEEK_END);
    file_size = ftell(tmp_file);
    rewind(tmp_file);

    //allocate memory for entire content
    file_content = calloc(1, (size_t)(file_size + 1));
    if(!file_content){
        remove(_tmp_file_name);
        fclose(tmp_file);
        fputs("Memory alloc fails.\n", stderr);
        return -1;
    }

    if(fread(file_content , (size_t)file_size, 1, tmp_file) < 0) return -1;

    _operation_results[_next_index] = file_content;
    _next_index++;

    //clean up memory, and usage
    fclose(tmp_file);
    remove(_tmp_file_name);

    return (int)(_next_index - 1);
}

void remove_operation_item(int index){
    if(index >= _next_index || index < 0 || _next_index <= 0) return;

    free(_operation_results[index]);
    for(int i = index; i < _next_index - 1; i++){
        _operation_results[i] = _operation_results[i + 1];
    }
    _next_index--;
}

void clear(){
    if(_next_index > 0){
        for(int i = 0; i < _next_index - 1; i++){
            free(_operation_results[i]);
        }
    }

    free(_operation_results);
}