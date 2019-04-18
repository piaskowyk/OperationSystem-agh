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
#include <sys/msg.h> 
#include <sys/ipc.h>

#include "server_const.h"

//obsłuch sygnałów to to to nie działa chyba

int mode = 0;
int runClient = 1;
int serverQueue, clientQueue;
int commandLen = 256;
struct message clientRequest, serverResponse;
int userID = -1;
pid_t pid;

void sender();
void catcher();

void stopCMD(struct StringArray* commandArgs);
void listCMD(struct StringArray* commandArgs);
void friendsCMD(struct StringArray* commandArgs);
void addCMD(struct StringArray* commandArgs);
void dellCMD(struct StringArray* commandArgs);
void echoCMD(struct StringArray* commandArgs);
void _2allCMD(struct StringArray* commandArgs);
void _2friendsCMD(struct StringArray* commandArgs);
void _2oneCMD(struct StringArray* commandArgs);

void executeFile(struct StringArray* commandArgs);

void endClient();
void handleSIGINT(int signalNumber);

int main(int argc, char *argv[], char *env[]) {

    char *homedir = getenv("HOME");

    key_t serverQueueKey;

    // create client queue for receiving messages from server
    if ((clientQueue = msgget(IPC_PRIVATE, 0660)) == -1) {
        printf("\033[1;33mClient:\033[0m Error while initialization client queue.\n");
        exit(1);
    }

    if ((serverQueueKey = ftok(homedir, PROJECT_ID)) == -1) {
        printf("\033[1;33mClient:\033[0m Error while get key from ftok().\n");
        exit(1);
    }

    if ((serverQueue = msgget(serverQueueKey, 0)) == -1) {
        printf("\033[1;33mClient:\033[0m Error while initialization access to server queue.\n");
        exit(1);
    }
    
    clientRequest.message_type = INIT;
    clientRequest.message_text.id = -1;
    sprintf(clientRequest.message_text.buf, "%d", clientQueue);

    // send message to server
    if (msgsnd(serverQueue, &clientRequest, sizeof(struct message_text), 0) == -1) {
        printf("\033[1;33mClient:\033[0m Error while sending INIT response to server.\n");
        exit(1);
    }
    else {
        printf("\033[1;33mClient:\033[0m Send INIT action to server.\n");
    }

    // read response from server
    if (msgrcv(clientQueue, &serverResponse, sizeof(struct message_text), 0, 0) == -1) {
        printf("\033[1;33mClient:\033[0m Error while read init response from server.\n");
        exit(1);
    }
    else {
        printf("\033[1;33mClient:\033[0m message received:\n\ttype: %ld, id: %d, message: %s \n", 
                serverResponse.message_type,
                serverResponse.message_text.id,
                serverResponse.message_text.buf
            );

        userID = serverResponse.message_type;
    }

    struct sigaction actionStruct;
    actionStruct.sa_handler = handleSIGINT;
    sigemptyset(&actionStruct.sa_mask); 
    sigaddset(&actionStruct.sa_mask, SIGINT); 
    actionStruct.sa_flags = 0;
    sigaction(SIGINT, &actionStruct, NULL);

    pid = fork();
    if(pid == 0){
        mode = 1;
        catcher();
    }
    else if(pid > 0) {
        mode = 2;
        sender();
    }
    else {
        fprintf(stderr, "\033[1;33mClient:\033[0m Error while creating fork.\n");
    }
    
    endClient();

    printf("\033[1;33mClient:\033[0m Close.\n");

    return 0;
}

//--------------------------------------------------------------------------------------------

void sendMessage() {
    if (msgsnd(serverQueue, &clientRequest, sizeof(struct message_text), 0) == -1) {
        printf("\033[1;33mClient:\033[0m Error while sending request to server.\n");
    }
    else {
        printf("\033[1;33mClient:\033[0m Send message to server.\n");
    }
}

int executeCommand(struct StringArray* commandArgs) {

    clientRequest.message_text.id = userID;
    
    if(strcmp(commandArgs->data[0], "STOP") == 0) {
        clientRequest.message_type = STOP;
        stopCMD(commandArgs);
    }
    else if(strcmp(commandArgs->data[0], "LIST") == 0) {
        if(commandArgs->size != 1) return 0;
        clientRequest.message_type = LIST;
        listCMD(commandArgs);
    }
    else if(strcmp(commandArgs->data[0], "FRIENDS") == 0) {
        if(commandArgs->size != 2) return 0;
        clientRequest.message_type = FRIENDS;
        friendsCMD(commandArgs);
    }
    else if(strcmp(commandArgs->data[0], "ADD") == 0) {
        if(commandArgs->size != 2) return 0;
        clientRequest.message_type = ADD;
        addCMD(commandArgs);
    }
    else if(strcmp(commandArgs->data[0], "DEL") == 0) {
        if(commandArgs->size != 2) return 0;
        clientRequest.message_type = DEL;
        dellCMD(commandArgs);
    }
    else if(strcmp(commandArgs->data[0], "ECHO") == 0) {
        if(commandArgs->size != 2) return 0;
        clientRequest.message_type = ECHO;
        echoCMD(commandArgs);
    }
    else if(strcmp(commandArgs->data[0], "2ALL") == 0) {
        if(commandArgs->size != 2) return 0;
        clientRequest.message_type = _2ALL;
        _2allCMD(commandArgs);
    }
    else if(strcmp(commandArgs->data[0], "2FRIENDS") == 0) {
        if(commandArgs->size != 2) return 0;
        clientRequest.message_type = _2FRIENDS;
        _2friendsCMD(commandArgs);
    }
    else if(strcmp(commandArgs->data[0], "2ONE") == 0) {
        if(commandArgs->size != 3) return 0;
        clientRequest.message_type = _2ONE;
        _2oneCMD(commandArgs);
    }
    else {
        return 0;
    }

    sendMessage();

    return 1;
}

//--------------------------------------------------------------------------------------------

void stopCMD(struct StringArray* commandArgs) {
    runClient = 0;
    sprintf(clientRequest.message_text.buf, "STOP from client %d", userID);
}

void listCMD(struct StringArray* commandArgs) {
    sprintf(clientRequest.message_text.buf, "LIST");
}

void friendsCMD(struct StringArray* commandArgs) {
    memcpy(clientRequest.message_text.buf, commandArgs->data[1], strlen(commandArgs->data[1]));
}

void addCMD(struct StringArray* commandArgs) {
    memcpy(clientRequest.message_text.buf, commandArgs->data[1], strlen(commandArgs->data[1]));
}

void dellCMD(struct StringArray* commandArgs) {
    memcpy(clientRequest.message_text.buf, commandArgs->data[1], strlen(commandArgs->data[1]));
}

void echoCMD(struct StringArray* commandArgs) {
    memcpy(clientRequest.message_text.buf, commandArgs->data[1], strlen(commandArgs->data[1]));
}

void _2allCMD(struct StringArray* commandArgs) {
    memcpy(clientRequest.message_text.buf, commandArgs->data[1], strlen(commandArgs->data[1]));
}

void _2friendsCMD(struct StringArray* commandArgs) {
    memcpy(clientRequest.message_text.buf, commandArgs->data[1], strlen(commandArgs->data[1]));
}

void _2oneCMD(struct StringArray* commandArgs) {
    memcpy(clientRequest.message_text.buf, commandArgs->data[1], strlen(commandArgs->data[1]));
    clientRequest.message_text.additionalArg = strtol(commandArgs->data[2], NULL, 0);
}

//--------------------------------------------------------------------------------------------

void sender() {
    //receive data from stdin
    while(runClient){
        char* command = calloc(commandLen, sizeof(char));  
        printf(">> ");
        int oneChar = 0; 
        char *ptr = command;
        while ((oneChar = fgetc(stdin)) != '\n') {
            if(command + commandLen > ptr) {
                (*ptr++) = (char)oneChar; 
            }
            else {
                command[commandLen-1] = '\0';
            }
        }

        struct StringArray commandArgs = explode(command, strlen(command), ' ');

        if(commandArgs.size < 1){
            printf("\033[1;33mClient:\033[0m Command not recognized.\n");
        }

        if(strcmp(commandArgs.data[0], "READ") == 0) {
            
        } 
        else {
            if(!executeCommand(&commandArgs)){
                printf("\033[1;33mClient:\033[0m Command not recognized.\n");
            }
        }

        free(commandArgs.data);
        free(command);
        sleep(1);
    }
}

void catcher() {
    while (1) {
        // read an incoming message, with priority order
        if (msgrcv(clientQueue, &serverResponse, sizeof(struct message_text), -1000, 0) == -1) {
            fprintf(stderr, "\033[1;33mClient:\033[0m Error while reading input data.\n");
        } else {
            printf("\033[1;33mClient:\033[0m message received:\n\ttype: %ld, id: %d, message: %s \n", 
                serverResponse.message_type, 
                serverResponse.message_text.id,
                serverResponse.message_text.buf
            );
        }

        if(serverResponse.message_type == SHUTDOWN){
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------

void executeFile(struct StringArray* commandArgs) {
    if(commandArgs->size != 2) {
        printf("\033[1;33mClient:\033[0m Invalid arguments.\n");
    }

    FILE* file = fopen(commandArgs->data[1], "r");
    if(file == NULL) {
        printf("\033[1;33mClient:\033[0m Error while opening file %s.\n", commandArgs->data[1]);
    }

    long fileSize = 0;
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    rewind(file);

    char* fileContent = calloc(fileSize, sizeof(char));
    struct StringArray command = explode(fileContent, fileSize, '\n');

    for(int i = 0; i < command.size; i++) {
        struct StringArray commandArgs = explode(command.data[i], strlen(command.data[i]), ' ');
        executeCommand(&commandArgs);
        free(commandArgs.data);
    }

    free(fileContent);
    free(command.data);
}

//--------------------------------------------------------------------------------------------

void endClient() {
    if (msgctl(clientQueue, IPC_RMID, NULL) == -1) {
        printf("\033[1;33mClient:\033[0m Error while closing client queue.\n");
        exit(1);
    }
    kill(SIGINT, pid);
}

void handleSIGINT(int signalNumber) {
    printf("\033[1;33mClient:\033[0m Receive signal SIGINT.\n");
    endClient();
}