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

#include "server_const.h"

struct StringArray{
    unsigned int size;
    char** data;
};

int nextClientID = 0;
int* clientsQueueId;

int* groupsSize;
int** friendsGroups;

int actualUserId = 0;

void executeCommand(struct message* input, struct message* output);

void stopCMD(struct message* input, struct message* output);
void listCMD(struct message* input, struct message* output);
void friendsCMD(struct message* input, struct message* output);
void initCMD(struct message* input, struct message* output);
void echoCMD(struct message* input, struct message* output);
void _2allCMD(struct message* input, struct message* output);
void _2friendsCMD(struct message* input, struct message* output);
void _2oneCMD(struct message* input, struct message* output);

int userIsExist(int userId);

int main(int argc, char *argv[], char *env[]) {


    // initialisation server
    char *homedir = getenv("HOME");
    clientsQueueId = calloc(MAX_CLIENTS_COUNT, sizeof(int));

    groupsSize = calloc(MAX_CLIENTS_COUNT, sizeof(int));
    friendsGroups = calloc(MAX_CLIENTS_COUNT, sizeof(int));
    for(int i = 0; i < MAX_CLIENTS_COUNT; i++){
        friendsGroups[i] = calloc(MAX_GROUP_SIZE, sizeof(int));
    }

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
        // read an incoming message, with priority order
        if (msgrcv(qid, &message, sizeof(struct message_text), -100, 0) == -1) {
            fprintf(stderr, "\033[1;32mServer:\033[0m Error while reading input data.\n");
        } else {
            printf("\033[1;32mServer:\033[0m message received:\n\ttype: %s, id: %d, message: %s \n", 
                typeToStr(message.message_type), 
                message.message_text.id,
                message.message_text.buf
            );
        }

        executeCommand(&message, &response);

        // send reply message to client
        if (
            userIsExist(actualUserId) && 
            msgsnd(clientsQueueId[actualUserId], &response, sizeof(struct message_text), 0) == -1
        ) {
            fprintf(stderr, "\033[1;32mServer:\033[0m Error while sending data, errno = %d\n", errno);
        }
        else {
            printf("\033[1;32mServer:\033[0m send response to client - %d.\n", 
                    actualUserId
                );
        }
        
    }

    return 0;
}

void executeCommand(struct message* input, struct message* output) {

    actualUserId = input->message_text.id - SHIFTID;

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

    output->message_text.id = actualUserId + SHIFTID;
    output->message_type = actualUserId + SHIFTID;
    
}

int userIsExist(int userId) {
    return userId < nextClientID;
}

struct StringArray explode(char* string, long len, char delimer) {
    struct StringArray itemsArray;
    char** items = NULL;
    int itemsCount = 0;

    itemsArray.size = 0;
    itemsArray.data = NULL;

    if(len == 0 || string == NULL) return itemsArray;

    itemsCount++;
    for(long i = 0; i < len; i++){
        if(string[i] == delimer) {
            itemsCount++;
        }
    }

    items = calloc(itemsCount, sizeof(char*));

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
        memcpy(items[i], string + indexStart, (indexGlob - indexStart) * sizeof(char));
        indexGlob++;
    }
    
    itemsArray.size = itemsCount;
    itemsArray.data = items;

    return itemsArray;
}

//------------------------------------------------------------------------------------------

void stopCMD(struct message* input, struct message* output) {

}

void listCMD(struct message* input, struct message* output) {
    char item[10];
    int offset = 0;
    for(int i = 0; i < nextClientID; i++){
        if(i != nextClientID - 1) sprintf(item, "%d, ", i);
        else sprintf(item, "%d", i);
        memcpy(output->message_text.buf + offset, item, strlen(item) * sizeof(char));
        offset += strlen(item);
    }
    *(output->message_text.buf + offset) = '\0';
}

void friendsCMD(struct message* input, struct message* output) {
    int userId = actualUserId;
    if(!userIsExist(actualUserId)){
        sprintf(output->message_text.buf, "User not exist.");
        return;
    }

    //explode ids list
    struct StringArray idList = explode(input->message_text.buf, 
                                        strlen(input->message_text.buf),
                                        ','
                                    );
    
    int groupSize = 0;
    for(int i = 0; i < idList.size && groupSize < 20; i++) {
        friendsGroups[userId][groupSize] = strtol(idList.data[i], NULL, 0);
        if(friendsGroups[userId][groupSize] >= 0 && userIsExist(friendsGroups[userId][groupSize])) groupSize++;
    }
    groupsSize[userId] = groupSize;

    sprintf(output->message_text.buf, "Create group with size %d.", groupSize);
}

void initCMD(struct message* input, struct message* output) {
    int userId = strtol(input->message_text.buf, NULL, 0);
    
    if(userIsExist(userId)){
        sprintf(output->message_text.buf, "User already exist.");
    }
    else {
        if(nextClientID < MAX_CLIENTS_COUNT){
            sprintf(output->message_text.buf, "%d", nextClientID);
            clientsQueueId[nextClientID] = userId;
            nextClientID++;
        }
        else {
            sprintf(output->message_text.buf, "Too many clients.");
        }
    }

    actualUserId = nextClientID-1;
    output->message_type = actualUserId;
}

void echoCMD(struct message* input, struct message* output) {
    __time_t now;
    time(&now);
    char date[21];
    strftime(date, 21, "%d-%m-%Y_%H:%M:%S", localtime(&now));
    sprintf(output->message_text.buf, "%s - %s", input->message_text.buf, date);
}

void _2allCMD(struct message* input, struct message* output) {

}

void _2friendsCMD(struct message* input, struct message* output) {

}

void _2oneCMD(struct message* input, struct message* output) {

}