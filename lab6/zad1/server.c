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

int nextClientID = 0;
int* clientIDs;

void executeCommand(struct message* input, struct message* output);

void stopCMD(struct message* input, struct message* output);
void listCMD(struct message* input, struct message* output);
void friendsCMD(struct message* input, struct message* output);
void initCMD(struct message* input, struct message* output);
void echoCMD(struct message* input, struct message* output);
void _2allCMD(struct message* input, struct message* output);
void _2friendsCMD(struct message* input, struct message* output);
void _2oneCMD(struct message* input, struct message* output);

int main(int argc, char *argv[], char *env[]) {

    // initialisation server
    char *homedir = getenv("HOME");
    clientIDs = calloc(MAX_CLIENTS_COUNT, sizeof(int));

    key_t msg_queue_key;
    int qid;
    struct message message, response;
    if ((msg_queue_key = ftok(homedir, PROJECT_ID)) == (key_t) -1) {
        fprintf(stderr, "\033[1;32mServer:\033[0m Error while getting unique queue key.\n");
        exit(100);
    }

    if ((qid = msgget(msg_queue_key, IPC_CREAT | QUEUE_PERMISSIONS)) == -1) {
        fprintf(stderr, "\033[1;32mServer:\033[0m Error while creating queue.\n");
        exit(101);
    }

    // start listening on user requests
    printf("\033[1;32mServer:\033[0m Server is running\n");

    while (1) {
        // read an incoming message
        if (msgrcv(qid, &message, sizeof(struct message_text), 0, 0) == -1) {
            fprintf(stderr, "\033[1;32mServer:\033[0m Error while reading input data.\n");
        } else {
            printf("\033[1;32mServer:\033[0m message received:\n\ttype: %lu, id: %d, message: %s \n", 
                message.message_type, 
                message.message_text.qid,
                message.message_text.buf
            );
        }

        executeCommand(&message, &response);

        // send reply message to client
        if (msgsnd(message.message_text.qid, &response, sizeof(struct message_text), 0) == -1) {
            fprintf(stderr, "\033[1;32mServer:\033[0m Error while sending data.\n");
        }
        else {
            printf("\033[1;32mServer:\033[0m response sent to client - %d.\n", 
                    message.message_text.qid
                );
        }
        
    }

    return 0;
}

void executeCommand(struct message* input, struct message* output) {
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
}

void stopCMD(struct message* input, struct message* output) {

}

void listCMD(struct message* input, struct message* output) {

}

void friendsCMD(struct message* input, struct message* output) {

}

void initCMD(struct message* input, struct message* output) {
    // response.message_type = 5;//message.message_text.qid;
    // response.message_text.qid = 0;
    // response.message_text.buf[0] = '\0';
}

void echoCMD(struct message* input, struct message* output) {

}

void _2allCMD(struct message* input, struct message* output) {

}

void _2friendsCMD(struct message* input, struct message* output) {

}

void _2oneCMD(struct message* input, struct message* output) {

}