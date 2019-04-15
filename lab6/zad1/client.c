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
    int commandLen = 20;

    key_t serverQueueKey;
    int serverQueue, clientQueue;
    struct message clientRequest, serverResponse;

    // create client queue for receiving messages from server
    if ((clientQueue = msgget(IPC_PRIVATE, 0660)) == -1) {
        perror("msgget: clientQueue");
        exit(1);
    }

    if ((serverQueueKey = ftok(homedir, PROJECT_ID)) == -1) {
        perror ("ftok");
        exit (1);
    }

    if ((serverQueue = msgget(serverQueueKey, 0)) == -1) {
        perror ("msgget: serverQueue");
        exit (1);
    }

    clientRequest.message_type = INIT;
    clientRequest.message_text.qid = clientQueue;
    clientRequest.message_text.buf[0] = '\0';

    // send message to server
    if (msgsnd(serverQueue, &clientRequest, sizeof(struct message_text), 0) == -1) {
        perror ("client: msgsnd");
        exit (1);
    }

    // read response from server
    if (msgrcv(clientQueue, &serverResponse, sizeof(struct message_text), 0, 0) == -1) {
        perror ("client: msgrcv");
        exit (1);
    }

    // process return message from server  
    printf("\033[1;33mClient:\033[0m message received:\n\ttype:%lu, id: %d, message: %s \n", 
            serverResponse.message_type, 
            serverResponse.message_text.qid,
            serverResponse.message_text.buf
        );

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
                command[19] = '\0';
            }
        }

        free(command);
    }
    
    // remove message queue
    if (msgctl(clientQueue, IPC_RMID, NULL) == -1) {
            perror("client: msgctl");
            exit(1);
    }

    printf ("Client: bye\n");

    return 0;
}