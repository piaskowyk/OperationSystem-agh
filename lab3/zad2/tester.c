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

int main(int argc, char *argv[]) {
    int validInput = 1;
    char* fileName;
    unsigned int intervalMin;
    unsigned int intervalMax;
    unsigned int howBytes;
    char *end;

    if(argc < 5){
        fputs("Not enough input arguments.\n", stderr);
        exit(100);
    }

    if(argc == 5){
        fileName = argv[1];
        intervalMin = (unsigned int)strtol(argv[2], &end, 0);
        intervalMax = (unsigned int)strtol(argv[3], &end, 0);
        howBytes = (unsigned int)strtol(argv[4], &end, 0);

        if(intervalMin >= intervalMax) validInput = 0;
    }
    else {
        validInput = 0;
    }

    if(!validInput){
        fputs("Invalid input arguments.\n", stderr);
        exit(101);
    }
    int limit = 2;
    while(limit > 0){
        limit--;
        printf("Save to file: %s\n", fileName);
        char* randomString = calloc(howBytes, sizeof(char));
        for(int i = 0; i < howBytes; i++){
            char c = (char)((rand()%25)+97);
            randomString[i] = c;
        }
        randomString[howBytes - 1] = '\n';

        int interval = (unsigned int)((rand()%(intervalMax - intervalMin)) + intervalMin);

        __time_t now; 
        time(&now);
        char date[22];
        strftime(date, 22, " %d-%m-%Y_%H:%M:%S  ", localtime(&now));
        FILE* testingFile = fopen(fileName, "a+");
        char pid[6];
        sprintf(pid, "%d", getpid());
        fwrite(pid, sizeof(int), 1, testingFile);
        fwrite(date, sizeof(char), 22, testingFile);
        fwrite(randomString, sizeof(char), howBytes, testingFile);
        fclose(testingFile);

        free(randomString);
        sleep(interval);
    }
    
    return 0;
}