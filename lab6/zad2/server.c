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

int nextClientID = 0;
mqd_t clientsQueueId[MAX_CLIENTS_COUNT];

int groupsSize[MAX_CLIENTS_COUNT];
int friendsGroups[MAX_CLIENTS_COUNT][MAX_GROUP_SIZE];

char input[MAX_MESSAGE_SIZE];
char output[MAX_MESSAGE_SIZE];

int actualUserId = 0;
int activeUserCount = 0;
int runServer = 1;

struct message inputMsg;
struct message outputMsg;

void executeCommand();
int userExist(int userId);
void sendShutdownToAllClients();
int sendMessage(int id, int type);

void stopCMD();
void listCMD();
void friendsCMD();
void addCMD();
void dellCMD();
void initCMD();
void echoCMD();
void _2allCMD();
void _2friendsCMD();
void _2oneCMD();

void handleSIGINT(int signalNumber);

int main(int argc, char *argv[], char *env[]) {
    // queue descriptors
    mqd_t serverQueue;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MESSAGE_SIZE;
    attr.mq_curmsgs = 0;

    for(int i = 0; i < MAX_CLIENTS_COUNT; i++){
        clientsQueueId[i] = -1;
        groupsSize[i] = -1;
    }

    if ((serverQueue = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        fprintf(stderr, "\033[1;32mServer:\033[0m Error while creating queue. e = %d\n", errno);
        exit(101);
    }
    
    struct sigaction actionStruct;
    actionStruct.sa_handler = handleSIGINT;
    sigemptyset(&actionStruct.sa_mask); 
    sigaddset(&actionStruct.sa_mask, SIGINT); 
    actionStruct.sa_flags = 0;
    sigaction(SIGINT, &actionStruct, NULL); 

    // start listening on user requests
    printf("\033[1;32mServer:\033[0m Server is running\n");

    while (runServer) {
        // get the oldest message with highest priority
        if (mq_receive(serverQueue, input, MAX_MESSAGE_SIZE, NULL) == -1) {
            fprintf(stderr, "\033[1;32mServer:\033[0m Error while reading input data. errno = %d\n", errno);
            continue;
        } 
        else {
            printf("\033[1;32mServer:\033[0m message received: %s \n", input);
            executeCommand();
        }

        if(inputMsg.message_type == STOP) continue;

        sendMessage(actualUserId, 0);        
    }

    //end working of server
    if (mq_close(serverQueue) == -1) {
        printf("\033[1;32mServer:\033[0m Error while closing server queue.\n");
    }

    if (mq_unlink(SERVER_QUEUE_NAME) == -1) {
        printf("\033[1;32mServer:\033[0m Error while remove server queue.\n");
    }

    printf("\033[1;32mServer:\033[0m Server close.\n");

    return 0;
}

void executeCommand() {
    //default value
    inputMsg.message_type = 0;
    //parsing inpit arguments do message struct
    struct StringArray msgArray = explode(input, strlen(input), '|');
    if(msgArray.size != 4) {
        printf("\033[1;32mServer:\033[0m Incorrect input data.\n");
        return;
    }

    inputMsg.message_type = strtol(msgArray.data[0], NULL, 0);
    inputMsg.message_text.id = strtol(msgArray.data[1], NULL, 0);
    inputMsg.message_text.additionalArg = strtol(msgArray.data[2], NULL, 0);
    memcpy(inputMsg.message_text.buf, msgArray.data[3], strlen(msgArray.data[3]));

    actualUserId = inputMsg.message_text.id;

    if(!userExist(actualUserId) && inputMsg.message_type != INIT){
        sprintf(outputMsg.message_text.buf, "User not exist.");
        outputMsg.message_text.id = SERVER_ID;
        outputMsg.message_type = ERROR;
        return;
    }

    switch (inputMsg.message_type) {
        case STOP: {
            stopCMD();
        } break;
        case LIST: {
            listCMD();
        } break;
        case FRIENDS: {
            friendsCMD();
        } break;
        case ADD: {
            addCMD();
        } break;
        case DEL: {
            dellCMD();
        } break;
        case INIT: {
            initCMD();
        } break;
        case ECHO: {
            echoCMD();
        } break;
        case _2ALL: {
            _2allCMD();
        } break;
        case _2FRIENDS: {
            _2friendsCMD();
        } break;
        case _2ONE: {
            _2oneCMD();
        } break;

        default:
            break;
    }

    outputMsg.message_text.id = SERVER_ID;
    outputMsg.message_type = actualUserId;

    free(msgArray.data);    
}

int userExist(int userId) {
    if(userId > nextClientID) return 0;
    return clientsQueueId[userId] != -1;
}

int userQueueExist(int userQueueId) {
    for(int i = 0; i < nextClientID; i++) {
        if(clientsQueueId[i] == userQueueId) return 1;
    }
    return 0;
}

void prepareMessage() {
    char tmp[1024];
    __time_t now;
    time(&now);
    char date[21];
    strftime(date, 21, "%d-%m-%Y_%H:%M:%S", localtime(&now));
    sprintf(tmp, "from %d, %s - %s", actualUserId, inputMsg.message_text.buf, date);
    memcpy(outputMsg.message_text.buf, tmp, 256);
    outputMsg.message_text.buf[255] = '\0';
}

int sendMessage(int id, int type) {

    sprintf(output, 
            "%ld|%d|%d|%s", 
            outputMsg.message_type, 
            outputMsg.message_text.id, 
            outputMsg.message_text.additionalArg, 
            outputMsg.message_text.buf
        );

    if (userExist(id) && mq_send(clientsQueueId[id], output, strlen (output) + 1, outputMsg.message_type) == -1) {
        fprintf(stderr, "\033[1;32mServer:\033[0m Error while sending data, errno = %d\n", errno);
        return -1;
    }
    else {
        if(type == 0){
            printf("\033[1;32mServer:\033[0m send response to client - %d.\n\n", actualUserId);
        }
        else if(type == 1) {
            printf("\033[1;32mServer:\033[0m send message to client %d from %d.\n", id, actualUserId);
        }
        else if(type == 2) {
            printf("\033[1;32mServer:\033[0m Send SHUTDOWN signal.\n");
        }
        return 1;
    }
}

int getFreeIndex() {
    if(nextClientID < MAX_CLIENTS_COUNT){
        nextClientID++;
        return nextClientID - 1;
    }
    else {
        for(int i = 0; i < MAX_CLIENTS_COUNT; i++){
            if(clientsQueueId[i] == -1) return i;
        }
    }
    return -1;
}

int existInGroup(int actualUserId, int friendsId) {
    for(int i = 0; i < groupsSize[actualUserId]; i++) {
        if(friendsGroups[actualUserId][i] == friendsId) return 1;
    }
    return 0;
}

//------------------------------------------------------------------------------------------

void sendShutdownToAllClients() {
    outputMsg.message_text.id = SERVER_ID;
    outputMsg.message_type = SHUTDOWN;
    sprintf(outputMsg.message_text.buf, "STOP");
    for(int i = 0; i < nextClientID; i++){
        if(!userExist(i)) continue;
        sendMessage(i, 2);
        if (mq_close(clientsQueueId[i]) == -1) {
            printf("\033[1;32mServer:\033[0m Error while closing server queue.\n");
        }
    }
}

void handleSIGINT(int signalNumber) {
    printf("\033[1;32mServer:\033[0m Receive signal SIGINT.\n");
    if(activeUserCount > 0){
        sendShutdownToAllClients();
    }
    else {
        printf("\033[1;32mServer:\033[0m Server close.\n");
        exit(0);
    }
}

//------------------------------------------------------------------------------------------

void stopCMD() {
    //remove queue
    clientsQueueId[actualUserId] = -1;
    
    // remove from any group
    for(int i = 0; i < nextClientID; i++) {
        for(int j = 0; j < groupsSize[i]; j++) {
            if(friendsGroups[i][j] == actualUserId) {
                for(int k = j; k < groupsSize[i] - 1; k++) {
                    friendsGroups[i][k] = friendsGroups[i][k + 1];
                }
                groupsSize[i]--;
                break;
            }
        }
    }

    sprintf(outputMsg.message_text.buf, "STOP - User is removed.");
    activeUserCount--;
    
    if(activeUserCount == 0) {
        runServer = 0;
    }
}

void listCMD() {
    char item[10];
    int offset = 0;
    for(int i = 0; i < nextClientID; i++){
        if(i != nextClientID - 1) sprintf(item, "%d, ", i);
        else sprintf(item, "%d", i);
        memcpy(outputMsg.message_text.buf + offset, item, strlen(item) * sizeof(char));
        offset += strlen(item);
    }
    *(outputMsg.message_text.buf + offset) = '\0';
}

void friendsCMD() {
    //explode ids list
    struct StringArray idList = explode(inputMsg.message_text.buf, strlen(inputMsg.message_text.buf), ',');
    
    if(idList.size > MAX_GROUP_SIZE){
        sprintf(outputMsg.message_text.buf, "FRIENDS - To many users.");
        return;
    }

    int groupSize = 0;;
    for(int i = 0; i < idList.size; i++) {
        friendsGroups[actualUserId][groupSize] = strtol(idList.data[i], NULL, 0);
        if(
            friendsGroups[actualUserId][groupSize] >= 0 && 
            userExist(friendsGroups[actualUserId][groupSize]) &&
            !existInGroup(actualUserId, friendsGroups[actualUserId][groupSize])
        ) {
            groupSize++;
        }
    }
    groupsSize[actualUserId] = groupSize;

    if(groupSize == 0) {
        sprintf(outputMsg.message_text.buf, "FRIENDS - Empty group.");
    }
    else {
        sprintf(outputMsg.message_text.buf, "FRIENDS - Create group with size %d.", groupsSize[actualUserId]);
    }
    free(idList.data);
}

void addCMD() {
    //explode ids list
    struct StringArray idList = explode(inputMsg.message_text.buf, strlen(inputMsg.message_text.buf), ',');
    
    int groupSize = groupsSize[actualUserId];
    if(idList.size + groupSize > MAX_GROUP_SIZE){
        sprintf(outputMsg.message_text.buf, "To many users.");
        return;
    }

    for(int i = 0; i < idList.size; i++) {
        friendsGroups[actualUserId][groupSize] = strtol(idList.data[i], NULL, 0);
        if(
            friendsGroups[actualUserId][groupSize] >= 0 && 
            userExist(friendsGroups[actualUserId][groupSize]) &&
            !existInGroup(actualUserId, friendsGroups[actualUserId][groupSize])
        ) {
            groupSize++;
        }
    }
    groupsSize[actualUserId] = groupSize;

    if(groupSize == 0) {
        sprintf(outputMsg.message_text.buf, "ADD - Empty group.");
    }
    else {
        sprintf(outputMsg.message_text.buf, "ADD - Create group with size %d.", groupsSize[actualUserId]);
    }
    free(idList.data);
}

void dellCMD() {
    int userId = actualUserId;
    //explode ids list
    struct StringArray idList = explode(inputMsg.message_text.buf, strlen(inputMsg.message_text.buf), ',');

    //remove grom group
    int userGroupId = 0;
    int groupIndex = groupsSize[userId];
    for(int i = 0; i < idList.size; i++) {
        userGroupId = strtol(idList.data[i], NULL, 0);
        for(int j = 0; j < groupIndex; j++) {
            if(friendsGroups[userId][j] == userGroupId){
                for(int k = j; k < groupIndex - 1; k++) {
                    friendsGroups[userId][k] = friendsGroups[userId][k + 1];
                }
                groupIndex--;
                break;
            }
        }
    }
    groupsSize[userId] = groupIndex;

    if(groupIndex == 0) {
        sprintf(outputMsg.message_text.buf, "DEL - Empty group.");
    }
    else {
        sprintf(outputMsg.message_text.buf, "DEL - New group size is %d.", groupsSize[userId]);
    }
    free(idList.data);
}

void initCMD() {
    mqd_t clientQueueId;

    if ((clientQueueId = mq_open(inputMsg.message_text.buf, O_WRONLY)) == 1) {
        sprintf(outputMsg.message_text.buf, "INIT - Unable to open client queue.");
        outputMsg.message_type = -1;
    }
    else {
        if(userQueueExist(clientQueueId)){
            sprintf(outputMsg.message_text.buf, "INIT - User already exist.");
            outputMsg.message_type = -1;
        }
        else {
            int index = getFreeIndex();
            if(index != -1){
                sprintf(outputMsg.message_text.buf, "%d", index);
                clientsQueueId[index] = clientQueueId;   
                actualUserId = index;
                outputMsg.message_type = index;
                activeUserCount++;
            }
            else {
                sprintf(outputMsg.message_text.buf, "INIT - Too many clients.");
                outputMsg.message_type = -1;
            }
        }
    }
}

void echoCMD() {
    char tmp[1024];
    __time_t now;
    time(&now);
    char date[21];
    strftime(date, 21, "%d-%m-%Y_%H:%M:%S", localtime(&now));
    sprintf(tmp, "%s - %s", inputMsg.message_text.buf, date);
    memcpy(outputMsg.message_text.buf, tmp, 256);
    outputMsg.message_text.buf[255] = '\0';
}

void _2allCMD() {
    prepareMessage();

    int sendCounter = 0;
    for(int i = 0; i < nextClientID; i++) {
        if(i != actualUserId && sendMessage(i, 1)) sendCounter++;
    }

    sprintf(outputMsg.message_text.buf, "2ALL - Send %d/%d message", sendCounter, nextClientID-1);
}

void _2friendsCMD() {
    prepareMessage();

    int sendCounter = 0;
    for(int i = 0; i < groupsSize[actualUserId]; i++) {
        if(friendsGroups[actualUserId][i] != actualUserId && sendMessage(friendsGroups[actualUserId][i], 1)) sendCounter++;
    }

    sprintf(outputMsg.message_text.buf, "2FRIENDS Send %d/%d message", sendCounter, groupsSize[actualUserId]);
}

void _2oneCMD() {
    prepareMessage();

    if(!userExist(inputMsg.message_text.additionalArg)){
        sprintf(outputMsg.message_text.buf, "2ONE - Destination user (%d) not exist", inputMsg.message_text.additionalArg);
    }
    else {
        sendMessage(inputMsg.message_text.additionalArg, 1);
        sprintf(outputMsg.message_text.buf, "2ONE - OK, send message");
    }

}