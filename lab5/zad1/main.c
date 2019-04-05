#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>

struct CommandArgs{
    unsigned int argCount;
    char* arguments;
};

int main(int argc, char *argv[], char *env[]) {
    int validInput = 1;
    struct CommandArgs* commands;
    unsigned int commandCount = 1;

    if(argc < 2){
        printf("Not enough arguments\n");
        exit(100);
    }

    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], (const char *)'|') == 0) commandCount++;
    }

    commands = calloc(commandCount, sizeof(struct CommandArgs));
    int argIndex = 0;
    for(int i = 0; i < commandCount; i++){

    }

    return 0;
}