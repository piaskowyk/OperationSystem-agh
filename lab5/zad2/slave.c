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

    srand(time(NULL));

    char* pathToPipe;
    unsigned int replyCount = 0;

    if(argc < 3) {
        fprintf(stderr, "Not enough arguments.\n");
        exit(100);
    }

    pathToPipe = argv[1];
    replyCount = (unsigned int)strtol(argv[2], NULL, 0);

    printf("\033[1;33mSlave:\033[0m PID: %d\n", getpid());

    //writing to pipe
    for (int i = 0; i < replyCount; i++) {
        sleep(rand() % 5 + 2);

        FILE* pipe = fopen(pathToPipe, "w+");
        if(pipe == NULL){
            fprintf(stderr, "\033[1;33mSlave:\033[0m unable to open pipe.\n");
            exit(101);
        }

        char* buffer = calloc(31, sizeof(char));
        FILE* fileDate = popen("date", "r");
        fread(buffer, sizeof(char), 30, fileDate);
        pclose(fileDate);

        char output[40];
        sprintf(output, "\t%d:\t%s", getpid(), buffer);

        fwrite(output, sizeof(char), 40, pipe);
        printf("\033[1;33mSlave:\033[0m %s", output);

        free(buffer);
        fclose(pipe);
    }

    return 0;
}