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
    char** arguments;
};

struct StringArray{
    unsigned int size;
    char** data;
    int* dataItemLen;
};

struct StringArray explode(char* string, long len, char delimer);
void cleanStringArray(struct StringArray items);
void cleanAll(
                struct StringArray items, 
                struct CommandArgs* commands, 
                unsigned int commandCount, 
                char* commandsFileContent,
                pid_t* process,
                int** pipeDescriptor
            );
void executeLine(char* line, int len);

int main(int argc, char *argv[], char *env[]) {

    char* pathToFileWithCommand;

    if(argc < 2){
        printf("Not enough arguments\n");
        exit(100);
    }

    pathToFileWithCommand = argv[1];

    //open file with command list
    FILE* commandsFile = fopen(pathToFileWithCommand, "r");
    if(!commandsFile){
        fprintf(stderr, "Unable to open file: %s\n", pathToFileWithCommand);
        exit(101);
    }

    //get size of file
    long fileSize = 0;
    fseek(commandsFile , 0L, SEEK_END);
    fileSize = ftell(commandsFile);
    rewind(commandsFile);

    if(fileSize < 1){
        fclose(commandsFile);
        fprintf(stderr, "Command file can not be empty.s\n");
        exit(102);
    }

    //get content of file
    char* commandsFileContent = calloc(fileSize, sizeof(char));
    if(commandsFileContent == NULL){
        fclose(commandsFile);
        fprintf(stderr, "Unable to allocate memory.\n");
        exit(103);
    }

    fread(commandsFileContent, sizeof(char), fileSize, commandsFile);
    fclose(commandsFile);

    //explode to line
    struct StringArray lines = explode(commandsFileContent, fileSize, '\n');
    for(int i = 0; i < lines.size; i++){
        executeLine(lines.data[i], lines.dataItemLen[i]);
    }

    free(commandsFileContent);
    cleanStringArray(lines);

    return 0;
}

struct StringArray explode(char* string, long len, char delimer) {
    struct StringArray itemsArray;
    char** items = NULL;
    int* itemsLen;
    int itemsCount = 0;

    itemsArray.size = 0;
    itemsArray.data = NULL;
    itemsArray.dataItemLen = NULL;

    if(len == 0 || string == NULL) return itemsArray;

    itemsCount++;
    for(long i = 0; i < len; i++){
        if(string[i] == delimer) {
            itemsCount++;
        }
    }

    items = calloc(itemsCount, sizeof(char*));
    itemsLen = calloc(itemsCount, sizeof(int));

    int indexGlob, indexStart;
    indexGlob = indexStart = 0;
    for(int i = 0; i < itemsCount; i++) {
        indexStart = indexGlob;
        while(indexGlob < len && string[indexGlob] != delimer) indexGlob++;

        if(indexGlob == indexStart){
            itemsCount--;
            i--;
            continue;
        }
        items[i] = calloc(indexGlob - indexStart + 1, sizeof(char));
        itemsLen[i] = indexGlob - indexStart;
        memcpy(items[i], string + indexStart, (indexGlob - indexStart) * sizeof(char));
        //move after space
        indexGlob++;
    }
    
    itemsArray.size = itemsCount;
    itemsArray.data = items;
    itemsArray.dataItemLen = itemsLen;

    return itemsArray;
}

void cleanStringArray(struct StringArray items) {
    for(int i = 0; i < items.size; i++) {
        free(items.data[i]);
    }
    free(items.data);
}

void cleanCommands(struct CommandArgs* commands, unsigned int commandCount) {
    for(int i = 0; i < commandCount; i++) {
        free(commands[i].arguments);
    }
    free(commands);
}

void cleanDestcriptors(int** pipeDestriptor, unsigned int commandCount) {
    for(int i = 0; i < commandCount - 1; i++) {
        free(pipeDestriptor[i]);
    }
    free(pipeDestriptor);
}

void cleanAll(
                struct StringArray items, 
                struct CommandArgs* commands, 
                unsigned int commandCount, 
                char* commandsFileContent, 
                pid_t* process,
                int** pipeDescriptor
            ) {
    cleanStringArray(items);
    cleanCommands(commands, commandCount);
    cleanDestcriptors(pipeDescriptor, commandCount);
    free(commandsFileContent);
    free(process);
}

void executeLine(char* line, int len) {

    struct CommandArgs* commands = NULL;
    unsigned int commandCount = 0;

    //parsing argument from file
    struct StringArray items = explode(line, len, ' ');

    commandCount++;
    for(long i = 0; i < items.size; i++){
        if(items.data[i] != NULL && strncmp(items.data[i], "|", 1) == 0) commandCount++;
    }

    if(commandCount > 10){
        fprintf(stderr, "Too many command in one file.\n");
        return;
    }

    commands = calloc(commandCount, sizeof(struct CommandArgs));
    if(commands == NULL){
        fprintf(stderr, "Unable to allocate memory.\n");
        return;
    }

    //pasting arguments into Commands structure
    int argIndex = 0;
    int startIndex = 0;
    for(long i = 0; i < commandCount; i++){
        startIndex = argIndex;
        while(argIndex < items.size && strncmp(items.data[argIndex], "|", 1) != 0){
            argIndex++;
        }

        commands[i].argCount = argIndex - startIndex;

        if(commands[i].argCount == 0){
            cleanCommands(commands, i);
            fprintf(stderr, "Command can not be null.\n");
            return;
        }

        commands[i].arguments = calloc(argIndex - startIndex + 1, sizeof(char*));
        for(int k = 0; k < commands[i].argCount; k++){
            commands[i].arguments[k] = items.data[startIndex + k];
        }
        argIndex++;
    }

    //start pipeline
    int** pipeDescriptor = calloc(commandCount - 1, sizeof(int*));
    for(int i = 0; i < commandCount - 1; i++) {
        pipeDescriptor[i] = calloc(2, sizeof(int));
        if(pipe(pipeDescriptor[i]) == -1){
            fprintf(stderr, "Unable to start new pipe\n");
            exit(107);
        }

    }

    pid_t* process = calloc((size_t)commandCount, sizeof(pid_t));

    for(int i = 0; i < commandCount; i++) {

        pid_t pid = fork();
        if(pid == 0) {

            //open and close descriptors, set descriptors mode
            if(i == 0) {//start pipeline
                close(pipeDescriptor[0][0]);
                dup2(pipeDescriptor[0][1], STDOUT_FILENO);

                execvp(commands[i].arguments[0], commands[i].arguments);

                for(int j = 1; j < commandCount; j++) {
                    close(pipeDescriptor[j][0]);
                    close(pipeDescriptor[j][1]);
                }
            }
            else if(i == commandCount - 1) {//end of pipeline
                close(pipeDescriptor[commandCount-2][1]);
                dup2(pipeDescriptor[commandCount-2][0], STDIN_FILENO);

                for(int j = 0; j < commandCount-2; j++) {
                    close(pipeDescriptor[j][0]);
                    close(pipeDescriptor[j][1]);
                }
            }
            else {
                for(int j = 0; j < commandCount - 1; j++) {
                    if(j == i || j == i-1) continue;
                    close(pipeDescriptor[j][0]);
                    close(pipeDescriptor[j][1]);
                }

                close(pipeDescriptor[i-1][1]);
                close(pipeDescriptor[i][0]);

                dup2(pipeDescriptor[i-1][0], STDIN_FILENO);
                dup2(pipeDescriptor[i][1], STDOUT_FILENO);
            }
            execvp(commands[i].arguments[0], commands[i].arguments);

        }
        else if(pid > 0){
            printf("Starting [%s] with pid: %d\n", commands[i].arguments[0], pid);
            process[i] = pid;
        }
    }

    printf("\nResults:\n");

    //close descriptors in parent process
    for(int i = 0; i < commandCount - 1; i++) {
        close(pipeDescriptor[i][0]);
        close(pipeDescriptor[i][1]);
    }

    for(int i = 0; i < commandCount; i++){
        pid_t pidProcess;
        int status;
        pidProcess = wait(&status);

        // if(i == 0) printf("\n");

        if(pidProcess == -1){
            fputs("Error while stopping process\n", stderr);
        }

        if(WEXITSTATUS(status) || WEXITSTATUS(status) == 0){
            // printf("End of process PID: %d\n", pidProcess);
        }
        else {
            printf("Error process %d, with code: %d\n", pidProcess, WEXITSTATUS(status));
        }
    }

    cleanCommands(commands, commandCount);
    cleanDestcriptors(pipeDescriptor, commandCount -1);
    cleanStringArray(items);
    free(process);
    printf("\n");
}