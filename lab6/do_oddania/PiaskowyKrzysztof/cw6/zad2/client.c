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
#include <errno.h>
#include <mqueue.h>

#include "server_const.h"

int mode = 0;
int runClient = 1;
int serverQueue, clientQueue;
int commandLen = 256;
struct message clientRequest, serverResponse;
int userID = -1;
pid_t pid;

char clientQueueName[64];
mqd_t serverQueue, clientQueue;

char input[MAX_MESSAGE_SIZE];
char output[MAX_MESSAGE_SIZE];

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

void parseServerResponse();
void parseClientRequest();

int main(int argc, char *argv[], char *env[]) {

    sprintf(clientQueueName, "/client-%d", getpid());

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MESSAGE_SIZE;
    attr.mq_curmsgs = 0;

    if ((clientQueue = mq_open(clientQueueName, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        printf("\033[1;33mClient:\033[0m Error while initialization client queue.\n");
        exit(1);
    }

    if ((serverQueue = mq_open(SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        printf("\033[1;33mClient:\033[0m Error while getting access to server queue.\n");
        exit(1);
    }

    clientRequest.message_type = INIT;
    clientRequest.message_text.id = -1;
    sprintf(clientRequest.message_text.buf, "%s", clientQueueName);

    parseClientRequest();

    // send message to server
    if (mq_send(serverQueue, output, strlen(output) + 1, INIT) == -1) {
        printf("\033[1;33mClient:\033[0m Error while sending INIT response to server.\n");
        exit(1);
    }
    
    // receive response from server
    if (mq_receive(clientQueue, input, MAX_MESSAGE_SIZE, NULL) == -1) {
        printf("\033[1;33mClient:\033[0m Error while read init response from server.\n");
        exit(1);
    }
    else {
        parseServerResponse();
        printf("\033[1;33mClient:\033[0m message received:\n\ttype: %ld, id: %d, message: %s \n", 
                serverResponse.message_type,
                serverResponse.message_text.id,
                serverResponse.message_text.buf
            );

        userID = serverResponse.message_type;
    }

    //create warcher and sender
    pid = fork();
    if(pid == 0){
        struct sigaction actionStruct;
        actionStruct.sa_handler = NULL; 
        actionStruct.sa_flags = 0;
        sigaction(SIGINT, &actionStruct, NULL);

        mode = 1;
        sender();
    }
    else if(pid > 0) {
        struct sigaction actionStruct;
        actionStruct.sa_handler = handleSIGINT;
        sigemptyset(&actionStruct.sa_mask); 
        sigaddset(&actionStruct.sa_mask, SIGINT); 
        actionStruct.sa_flags = 0;
        sigaction(SIGINT, &actionStruct, NULL);

        mode = 2;
        catcher();
    }
    else {
        fprintf(stderr, "\033[1;33mClient:\033[0m Error while creating fork.\n");
    }
    
    endClient();

    return 0;
}

//--------------------------------------------------------------------------------------------

void sendMessage() {
    parseClientRequest();
    if (mq_send(serverQueue, output, strlen(output) + 1, clientRequest.message_type) == -1) {
        printf("\033[1;33mClient:\033[0m Error while sending request to server.\n");
    }
    else {
        printf("\033[1;33mClient:\033[0m Send message %s to server.\n", typeToStr(clientRequest.message_type));
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
        if(commandArgs->size != 1 && commandArgs->size != 2) return 0;
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

void simpleAction(struct StringArray* commandArgs, int index) {
    memcpy(clientRequest.message_text.buf, commandArgs->data[index], strlen(commandArgs->data[index]));
    clientRequest.message_text.buf[strlen(commandArgs->data[index])] = '\0';
}

//--------------------------------------------------------------------------------------------

void stopCMD(struct StringArray* commandArgs) {
    runClient = 0;
    simpleAction(commandArgs, 0);
}

void listCMD(struct StringArray* commandArgs) {
    simpleAction(commandArgs, 0);
}

void friendsCMD(struct StringArray* commandArgs) {
    if(commandArgs->size == 2) {
        simpleAction(commandArgs, 1);
    }
    else {
        clientRequest.message_text.buf[0] = '\0';
    }
}

void addCMD(struct StringArray* commandArgs) {
    simpleAction(commandArgs, 1);
}

void dellCMD(struct StringArray* commandArgs) {
    simpleAction(commandArgs, 1);
}

void echoCMD(struct StringArray* commandArgs) {
    simpleAction(commandArgs, 1);
}

void _2allCMD(struct StringArray* commandArgs) {
    simpleAction(commandArgs, 1);
}

void _2friendsCMD(struct StringArray* commandArgs) {
    simpleAction(commandArgs, 1);
}

void _2oneCMD(struct StringArray* commandArgs) {
    simpleAction(commandArgs, 2);
    clientRequest.message_text.additionalArg = strtol(commandArgs->data[1], NULL, 0);
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
            executeFile(&commandArgs);
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
        if (mq_receive(clientQueue, input, MAX_MESSAGE_SIZE, NULL) == -1) {
            printf("\033[1;33mClient:\033[0m Error while read init response from server.\n");
            continue;
        }
        else {
            parseServerResponse();
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
    fread(fileContent, fileSize, sizeof(char), file);

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

    clientRequest.message_text.id = userID;
    clientRequest.message_type = STOP;
    sprintf(clientRequest.message_text.buf, "STOP from client %d", userID);

    parseClientRequest();

    if (mq_send(serverQueue, output, strlen(output) + 1, clientRequest.message_type) == -1) {
        fprintf(stderr, "\033[1;33mClient:\033[0m Error while sending data about STOP.\n");
    }
    else {
        printf("\033[1;33mClient:\033[0m Send information about STOP.\n");
    }


    if (mq_close(serverQueue) == -1) {
        printf("\033[1;33mClient:\033[0m Error while closing server queue.\n");
    }

    if (mq_close(clientQueue) == -1) {
        printf("\033[1;33mClient:\033[0m Error while closing client queue.\n");
    }

    if (mq_unlink(clientQueueName) == -1) {
        printf("\033[1;33mClient:\033[0m Error while remove client queue.\n");
    }

    kill(pid, 9);
    printf("\033[1;33mClient:\033[0m Close.\n");
    exit(0);
}

void handleSIGINT(int signalNumber) {
    printf("\033[1;33mClient:\033[0m Receive signal SIGINT.\n");
    endClient();
}

//----------------------------------------------------------------------------------------------------------------

void parseServerResponse() {

    struct StringArray msgArray = explode(input, strlen(input), '|');

    if(msgArray.size > 0) serverResponse.message_type = strtol(msgArray.data[0], NULL, 0);
    else serverResponse.message_type = ERROR;

    if(msgArray.size > 1) serverResponse.message_text.id = strtol(msgArray.data[1], NULL, 0);
    else serverResponse.message_text.id = ERROR;

    if(msgArray.size > 2) serverResponse.message_text.additionalArg = strtol(msgArray.data[2], NULL, 0);
    else serverResponse.message_text.additionalArg = ERROR;

    if(msgArray.size > 3) {
        int len = strlen(msgArray.data[3]);
        memcpy(serverResponse.message_text.buf, msgArray.data[3], len);
        serverResponse.message_text.buf[len] = '\0';
    }
    else {
        serverResponse.message_text.buf[0] = '\0';
    }

    free(msgArray.data);
}

void parseClientRequest() {
    sprintf(output, 
        "%ld|%d|%d|%s", 
        clientRequest.message_type, 
        clientRequest.message_text.id, 
        clientRequest.message_text.additionalArg, 
        clientRequest.message_text.buf
    );
}