#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <wait.h>

#define __USE_XOPEN
#include <time.h>

struct Search_cmd{
    char* path;
    char type;
    char date[20];
};

int search(struct Search_cmd search_cmd);
char* recognized_type(__mode_t type);
char* abs_to_rel(char* path);
char* main_path;

int main(int argc, char *argv[], char *env[]) {
    int valid_input = 1;

    if(argc == 5){
        struct Search_cmd search_cmd;
        search_cmd.path = realpath(argv[1], NULL);
        main_path = realpath(argv[1], NULL);

        if(!search_cmd.path || !main_path){
            fputs("Path to directory not exist.\n", stderr);
            exit(102);
        }

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
            search(search_cmd);
        }

        free(search_cmd.path);
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

int search(struct Search_cmd search_cmd) {
    DIR* dir_handler;
    struct dirent *dir;
    dir_handler = opendir(search_cmd.path);

    if (dir_handler) {
        chdir(search_cmd.path);

        while ((dir = readdir(dir_handler)) != NULL) {

            if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0){
                continue;
            }

            struct stat file_info;
            stat(dir->d_name, &file_info);

            struct tm ime_struct;
            strptime(search_cmd.date, "%d/%m/%Y %H:%M:%S", &ime_struct);
            time_t time_struct = mktime(&ime_struct);

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

            char* path = realpath(dir->d_name, NULL);
            if(!path) continue;
            
            char* type_name = recognized_type(file_info.st_mode);

            if(showInfo){
                char date_last_access[20];
                char date_last_modification[20];

                strftime(date_last_access, 20, "%d/%m/%Y %H:%M:%S", localtime(&file_info.st_atime));
                strftime(date_last_modification, 20, "%d/%m/%Y %H:%M:%S", localtime(&file_info.st_mtime));

                printf("Path: %s\n", path);
                printf("Name: %s, Type: %s, Size: %lu B, Date of last access: %s, Date of last modification: %s\n\n",
                    dir->d_name,
                    type_name,
                    file_info.st_size,
                    date_last_access,
                    date_last_modification);
            }

            if((file_info.st_mode & S_IFMT) == S_IFDIR){
                char* patch_copy = search_cmd.path;
                search_cmd.path = path;

                pid_t pid;
                pid = fork();
                if (pid != 0) {
                    wait(NULL);
                    printf("\n");
                    search(search_cmd);
                }
                else {
                    char* copy_path = calloc(strlen(path), sizeof(char));
                    memcpy(copy_path, path, strlen(path) * sizeof(char));
                    printf("Process PPID: %d, PID: %d (ls -l): %s\n", getppid(), getpid(), abs_to_rel(patch_copy));
                    free(copy_path);
                    execl("/bin/ls", "ls", "-l", path, NULL);
                }

                search_cmd.path = patch_copy;
                chdir(search_cmd.path);
            }
            
            free(path);
        }
        closedir(dir_handler);
    }

    return 1;
}

char* recognized_type(__mode_t type){
    char* type_name;
    switch (type & S_IFMT) {
        case S_IFBLK: { type_name = "block device"; } break;
        case S_IFCHR: { type_name = "character device"; } break;
        case S_IFDIR: { type_name = "directory"; } break;
        case S_IFIFO: { type_name = "FIFO/pipe"; } break;
        case S_IFLNK: { type_name = "symlink"; } break;
        case S_IFREG: { type_name = "regular file"; } break;
        case S_IFSOCK: { type_name = "socket"; } break;
        default: { type_name = "unknown"; } break;
    }

    return type_name;
}

char* abs_to_rel(char* path){
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
