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

int main(int argc, char *argv[], char *env[]) {

    char *homedir = getenv("HOME");
    // int commandLen = 20;

    key_t serverQueueKey;
    int serverQueue, clientQueue;
    struct message clientRequest, serverResponse;
    int userID = 0;

    // create client queue for receiving messages from server
    if ((clientQueue = msgget(IPC_PRIVATE, 0660)) == -1) {
        printf("\033[1;33mClient:\033[0m Error while initialization client queue.\n");
        exit(1);
    }

    if ((serverQueueKey = ftok(homedir, PROJECT_ID)) == -1) {
        printf("\033[1;33mClient:\033[0m Error while get key from ftok().\n");
        exit (1);
    }

    if ((serverQueue = msgget(serverQueueKey, 0)) == -1) {
        printf("\033[1;33mClient:\033[0m Error while initialization access to server queue.\n");
        exit (1);
    }
    
    clientRequest.message_type = INIT;
    clientRequest.message_text.id = -1;
    sprintf(clientRequest.message_text.buf, "%d", clientQueue);

    // send message to server
    if (msgsnd(serverQueue, &clientRequest, sizeof(struct message_text), 0) == -1) {
        printf("\033[1;33mClient:\033[0m Error while send init response to server.\n");
        exit (1);
    }

    // read response from server
    if (msgrcv(clientQueue, &serverResponse, sizeof(struct message_text), 0, 0) == -1) {
        printf("\033[1;33mClient:\033[0m Error while read init response from server.\n");
        exit (1);
    }
    // process return message from server  
    printf("\033[1;33mClient:\033[0m message received:\n\ttype: %ld, message: %s \n", 
            serverResponse.message_type,
            serverResponse.message_text.buf
        );

    userID = serverResponse.message_type;

    //################################################################################################

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

    //################################################################################################

    // while(1){
    //     char* command = calloc(commandLen, sizeof(char));  
    //     printf(">> ");
    //     int oneChar = 0; 
    //     char *ptr = command;
    //     while ((oneChar = fgetc(stdin)) != '\n') {
    //         if(command + commandLen > ptr) {
    //             (*ptr++) = (char)oneChar; 
    //         }
    //         else {
    //             command[19] = '\0';
    //         }
    //     }

    //     free(command);
    // }
    
    // remove message queue
    if (msgctl(clientQueue, IPC_RMID, NULL) == -1) {
            printf("\033[1;33mClient:\033[0m Error while closing client queue.\n");
            exit(1);
    }

    // printf("\033[1;33mClient:\033[0m Close.\n");

    return 0;
}