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

int main(int argc, char *argv[], char *env[]) {

    char* pathToPipe;

    if(argc < 2) {
        fprintf(stderr, "Not enough arguments.\n");
        exit(100);
    }

    pathToPipe = argv[1];

    //creating naming pipe
    mkfifo(pathToPipe, 0666);

    FILE* pipe = fopen(pathToPipe, "r");
    while(1){
        char* buffer = calloc(41, sizeof(char));

        while (fread(buffer, sizeof(char), 40, pipe) > 0){
            printf("\033[1;32mMaster:\033[0m%s", buffer);
        }

        free(buffer);
    }

    return 0;
}