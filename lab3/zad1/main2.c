#define _XOPEN_SOURCE 500
#define __USE_XOPEN
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ftw.h>
#include <time.h>
#include <libgen.h>
#include <wait.h>

struct Search_cmd{
    char type;
    char date[20];
};

char* recognized_type(int type);
char* abs_to_rel(char* path);
struct Search_cmd search_cmd;
int print_info(const char *name, const struct stat *status, int type, struct FTW*);
char* main_path;

int main(int argc, char *argv[]) {
    int valid_input = 1;

    if(argc == 5){

        if(strncmp(argv[2], "<", 1) == 0) {
            search_cmd.type = -1;
        }
        else if(strncmp(argv[2], "=", 1) == 0) {
            search_cmd.type = 0;
        }
        else if(strncmp(argv[2], ">", 1) == 0) {
            search_cmd.type = 1;
        }
        else {
            valid_input = 0;
        };

        memcpy(search_cmd.date, argv[3], 10);
        search_cmd.date[10] = ' ';
        memcpy(search_cmd.date + 11, argv[4], 8);
        search_cmd.date[19] = '\0';

        if(valid_input){
            main_path = realpath(argv[1], NULL);
            if(!main_path){
                fputs("Path to directory not exist.\n", stderr);
                exit(102);
            }
            nftw(argv[1], print_info, 100, FTW_PHYS);
        }

        free(main_path);
    }
    else {
        valid_input = 0;
        fputs("Invalid input arguments.\n", stderr);
    }

    if(!valid_input){
        exit(101);
    }

    return 0;
}

//##########################################################################################

int print_info(const char *name, const struct stat *status, int type, struct FTW* ftw_struct) {

    if(strcmp(".", name) == 0 || strcmp("..", name) == 0) return 0;

    struct stat file_info;
    stat(name, &file_info);

    struct tm time_formatter;
    strptime(search_cmd.date, "%d/%m/%Y %H:%M:%S", &time_formatter);
    time_t time_struct = mktime(&time_formatter);

    int showInfo = 1;
    switch(search_cmd.type){
        case -1:{
            if(time_struct <= file_info.st_mtime){
                showInfo = 0;
            }
        }break;
        case 0:{
            if(time_struct != file_info.st_mtime){
                showInfo = 0;
            }
        }break;
        case 1:{
            if(time_struct >= file_info.st_mtime){
                showInfo = 0;
            }
        }break;
        default: break;
    }

    char* type_name = recognized_type(type);

    if(showInfo){
        char date_last_access[20];
        char date_last_modification[20];

        strftime(date_last_access, 20, "%d/%m/%Y %H:%M:%S", localtime(&file_info.st_atime));
        strftime(date_last_modification, 20, "%d/%m/%Y %H:%M:%S", localtime(&file_info.st_mtime));

        char* copy_name = calloc(strlen(name) + 1, sizeof(char));
        memcpy(copy_name, name, strlen(name) + 1);

        printf("Path: %s\n", realpath(name, NULL));
        printf("Name: %s, Type: %s, Size: %lu B, Date of last access: %s, Date of last modification: %s\n\n",
            basename(copy_name),
            type_name,
            file_info.st_size,
            date_last_access,
            date_last_modification);
        free(copy_name);
    }

    if((file_info.st_mode & S_IFMT) == S_IFDIR){
        pid_t pid;
        pid = fork();
        if (pid != 0){
            wait(NULL);
            printf("\n");
        }
        else {
            char* copy_path = calloc(strlen(name), sizeof(char));
            memcpy(copy_path, name, strlen(name) * sizeof(char));
            printf("Process PPID: %d, PID: %d (ls -l): %s\n", getppid(), getpid(), abs_to_rel(realpath(copy_path, NULL)));
            free(copy_path);
            execl("/bin/ls", "ls", "-l", name, NULL);
        }
    }

    return 0;
}

char* recognized_type(int type){
    char* type_name;
    switch (type) {
        case FTW_F: { type_name = "regular file"; } break;
        case FTW_D: { type_name = "directory"; } break;
        case FTW_DNR: { type_name = "directory that could not be read"; } break;
        case FTW_SL: { type_name = "symbolic link"; } break;
        default: { type_name = "unknown"; } break;
    }

    return type_name;
}


char* abs_to_rel(char* path){
    if(!path) return "unresolved";

    int diff = (int)(strlen(path) - strlen(main_path));
    int main_path_len = (int)strlen(main_path);
    if(diff > 0) {
        for(int i = 0; i < main_path_len; i++){
            path[i] = path[i + main_path_len +1];
        }

        for(int i = (int)strlen(main_path); i < strlen(path); i++){
            path[i] = '\0';
        }
    }

    return path;
}