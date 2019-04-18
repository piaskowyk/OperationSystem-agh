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

int mode = 0;
int serverQueue, clientQueue;
int commandLen = 256;
struct message clientRequest, serverResponse;
int userID = -1;

void sender();
void catcher();

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

    pid_t pid = fork();
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
    
    //send STOP signal
    kill(SIGINT, pid);

    printf("\033[1;33mClient:\033[0m Close.\n");

    return 0;
}

//--------------------------------------------------------------------------------------------

void executeCommand(struct message* input, struct message* output) {

    actualUserId = input->message_text.id - SHIFTID;

    if(!userExist(actualUserId) && input->message_type != INIT){
        sprintf(output->message_text.buf, "User not exist.");
        output->message_text.id = SERVER_ID;
        output->message_type = 500;
        return;
    }

    switch (input->message_type) {
        case STOP: {
            stopCMD(input, output);
        } break;
        case LIST: {
            listCMD(input, output);
        } break;
        case FRIENDS: {
            friendsCMD(input, output);
        } break;
        case INIT: {
            initCMD(input, output);
        } break;
        case ECHO: {
            echoCMD(input, output);
        } break;
        case _2ALL: {
            _2allCMD(input, output);
        } break;
        case _2FRIENDS: {
            _2friendsCMD(input, output);
        } break;
        case _2ONE: {
            _2oneCMD(input, output);
        } break;

        default:
            break;
    }

    output->message_text.id = SERVER_ID;
    output->message_type = actualUserId + SHIFTID;
    
}

//--------------------------------------------------------------------------------------------

void sender() {
    while(1){
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
        printf("inser: %s\n",command);

        struct StringArray commandArgs = explode(command, strlen(command), ' ');

        if(commandArgs.size < 1){
            printf("\033[1;33mClient:\033[0m Command not recognized.\n");
        }



        free(commandArgs.data);
        free(command); 
    }


    clientRequest.message_type = FRIENDS;
    clientRequest.message_text.id = userID;
    sprintf(clientRequest.message_text.buf, "0,123,456,7890");

    // send message to server
    if (msgsnd(serverQueue, &clientRequest, sizeof(struct message_text), 0) == -1) {
        printf("\033[1;33mClient:\033[0m Error while send init response to server.\n");
        exit (1);
    }
    else {
        fprintf(stderr, "\033[1;33mClient:\033[0m Send data to server.\n");
    }

    // read response from server
    if (msgrcv(clientQueue, &serverResponse, sizeof(struct message_text), 0, 0) == -1) {
        printf("\033[1;33mClient:\033[0m Error while read init response from server.\n");
        exit (1);
    }
    else {
        printf("\033[1;33mClient:\033[0m message received:\n\ttype: %ld, message: %s \n", 
                serverResponse.message_type,
                serverResponse.message_text.buf
            );
    } 
}

void catcher() {
    //odbieraj sygnały z kolejki servera
}

void handleSIGINT(int signalNumber) {
    printf("\033[1;33mClient:\033[0m Receive signal SIGINT.\n");
    if (msgctl(clientQueue, IPC_RMID, NULL) == -1) {
        printf("\033[1;33mClient:\033[0m Error while closing client queue.\n");
        exit(1);
    }
}