#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#include "utils.h"
#include "list.h"

#define MAX_NAME_LEN 10
#define LOCAL_CONNECTION 1
#define INTERNET_CONNECTION 2

char* clientName;
int connectionType;
char* connectionAddress;
int running = 1;
int server = -1;

void sendMessage(int socket, const struct ClientMessage *message);
struct ServerMessage getMessage(int socket);

void handleSIGINT();
void *threadWork(void * data);

void handleRequest(int socket, struct ServerMessage * message);

struct ClientMessage workAction(struct ServerMessage *message);
struct ClientMessage pingAction(struct ServerMessage * message);

int main(int argc, char *argv[], char *env[]) {

    //signal handler init
    struct sigaction actionStruct;
    actionStruct.sa_handler = handleSIGINT;
    sigemptyset(&actionStruct.sa_mask);
    sigaddset(&actionStruct.sa_mask, SIGINT);
    actionStruct.sa_flags = 0;
    sigaction(SIGINT, &actionStruct, NULL);

    if(argc < 4){
        printErrorMessage("Not enough arguments", 1);
    }

    clientName = argv[1];
    if(strcmp(clientName, "") == 0){
        printErrorMessage("Client name can not by empty", 1);
    }

    connectionType = strtol(argv[2], NULL, 0);
    if(connectionType != 1 && connectionType != 2) {
        printErrorMessage("Connection type can be only 1 or 2", 1);
    }

    connectionAddress = argv[3];
    if(strcmp(connectionAddress, "") == 0 || strlen(connectionAddress) > 255){
        printErrorMessage("Connection address is invalid", 1);
    }

    //creating sockets connection
    int socketFd;
    struct sockaddr_un socketLocalConf;
    struct sockaddr_in socketInternetConf;

    memset(&socketLocalConf, 0, sizeof(socketLocalConf));

    if(connectionType == LOCAL_CONNECTION) {
        socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (socketFd == -1) {
            printErrorMessage("Local socket creation failed", 3);
        }
        else {
            printf("Create local socket\n");
        }

        socketLocalConf.sun_family = AF_UNIX;
        strcpy(socketLocalConf.sun_path, connectionAddress);

        if (connect(socketFd, (struct sockaddr *)&socketLocalConf, sizeof(socketLocalConf)) != 0) {
            printErrorMessage("Unable connect to local socket", 3);
        }
        else{
            printf("Connect to local socket\n");
        }
    }
    else {
        struct StringArray connection = explode(
                connectionAddress,
                strlen(connectionAddress),
                ':');

        if(connection.size != 2) {
            printErrorMessage("Invalid IP address", 4);
        }

        socketFd = socket(AF_INET, SOCK_STREAM, 0);
        if (socketFd == -1) {
            printErrorMessage("Internet socket creation failed", 4);
        }
        else {
            printf("Create internet socket\n");
        }

        socketInternetConf.sin_family = AF_INET;
        socketInternetConf.sin_addr.s_addr = inet_addr(connection.data[0]);
        uint16_t port = strtol(connection.data[1], NULL, 0);
        socketInternetConf.sin_port = htons(port);

        if ((bind(socketFd, (struct sockaddr *)&socketInternetConf, sizeof(socketInternetConf))) != 0) {
            printErrorMessage("Internet socket bind failed", 4);
        }
        else {
            printf("Connect to internet socket\n");
        }

        if (connect(socketFd, (struct sockaddr *)&socketInternetConf, sizeof(socketInternetConf)) != 0) {
            printErrorMessage("Unable connect to internet socket", 4);
        }
        else{
            printf("Connect to internet socket\n");
        }

        cleanStringArray(&connection);
    }

    server = socketFd;

    //send client name
    struct ClientMessage messageStart = {REGISTER_ACTION, 0, strlen(clientName), "", clientName};
    messageStart.type = REGISTER_ACTION;
    messageStart.dataLen = 0;
    messageStart.data = NULL;
    messageStart.clientNameLen = strlen(clientName);
    messageStart.clientName = calloc(messageStart.clientNameLen, sizeof(char));
    memcpy(messageStart.clientName, clientName, messageStart.clientNameLen);

    sendMessage(socketFd, &messageStart);
    cleanClientMessage(&messageStart);

    while(running) {
//        printf("Waiting...\n");

        struct ServerMessage message = getMessage(socketFd);
        handleRequest(socketFd, &message);
        cleanServerMessage(&message);
    }

    close(socketFd);
    printf("END\n");
}

void sendMessage(int socket, const struct ClientMessage *message) {
    write(socket, &message->type, sizeof(message->type));
    write(socket, &message->dataLen, sizeof(message->dataLen));
    write(socket, &message->clientNameLen, sizeof(message->clientNameLen));
    if(message->dataLen > 0) {
        write(socket, message->data, message->dataLen * sizeof(char));
    }
    if(message->clientNameLen > 0) {
        write(socket, message->clientName, message->clientNameLen * sizeof(char));
    }
}

struct ServerMessage getMessage(int socket) {
    struct ServerMessage message;

    if(read(socket, &message.code, sizeof(int)) != sizeof(message.code)){
        printf("Error while reading data 1 e:%d\n", errno);
    }
//    printf("c: %d\n", message.code);

    if(read(socket, &message.type, sizeof(int)) != sizeof(message.type)){
        printf("Error while reading data 2 e:%d\n", errno);
    }
//    printf("t: %d\n", message.type);

    if(read(socket, &message.dataLen, sizeof(int)) != sizeof(message.dataLen)){
        printf("Error while reading data 3 e:%d\n", errno);
    }
//    printf("l: %d\n", message.dataLen);

    if(message.dataLen > 0) {
        message.data = calloc(message.dataLen + 1, sizeof(char));
        if(message.data == NULL){
            printErrorMessage("Unable to allocate memory", 5);
        }

        if(read(socket, message.data, message.dataLen) != message.dataLen) {
            printf("Error while reading data 4 e:%d\n", errno);
        }
    }
    else {
        message.data = NULL;
    }

    return message;
}

void handleSIGINT() {
    if(server < 0) {
        return;
    }
    printf("Receive signal SIGINT.\n");
    struct ClientMessage message;
    message.dataLen = 0;
    message.data = NULL;
    message.type = LOGOUT_ACTION;
    message.clientNameLen = strlen(clientName);
    message.clientName = calloc(message.clientNameLen, sizeof(char));
    memcpy(message.clientName, clientName, message.clientNameLen);

    sendMessage(server, &message);

    exit(0);
}

void handleRequest(int socket, struct ServerMessage * message) {

    printf("Handle: %d\n", message->type);
    struct ClientMessage response;

    switch(message->type){
        case WORK_ACTION: {
            workAction(message);
            return;
        }
        case PING_ACTION: {
            response = pingAction(message);
        }
        break;
        default:
            return;
    }

    sendMessage(socket, &response);
}

struct ClientMessage pingAction(struct ServerMessage * message) {
//    printf("PING action\n");

    struct ClientMessage response;

    response.clientName = calloc(strlen(clientName), sizeof(char));
    memcpy(response.clientName, clientName, strlen(clientName));
    response.clientNameLen = strlen(clientName);
    response.type = PING_ACTION;
    response.dataLen = 0;
    response.data = NULL;

    return response;
}
int create = 0;
struct ClientMessage workAction(struct ServerMessage *message) {
    create = 0;
//    printf("ADS_data:%s\n", message->data);
    pthread_t workThread;
    if(pthread_create(&workThread, NULL, threadWork, (void *)message) != 0) {
        printErrorMessage("Unable to create work thread", 6);
    }

    if(pthread_detach(workThread) != 0) {
        printErrorMessage("Unable to detach work thread", 6);
    }
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 50;
    while (!create){
        nanosleep(&tim , &tim2);
    }
}

void *threadWork(void * data) {
    struct ClientMessage response;

    response.clientName = calloc(strlen(clientName), sizeof(char));
    memcpy(response.clientName, clientName, strlen(clientName));
    response.clientNameLen = strlen(clientName);
    response.type = WORK_ACTION;

    struct ServerMessage *message = (struct ServerMessage *)data;
    create = 1;
//    printf("size: %ld\n", sizeof(message) + message->dataLen);
//    printf("data:%s\n", message->data);

//    return NULL;
    struct StringArray allWords = explode(message->data, message->dataLen, ' ');
//printf("słowa: %d\n",allWords.size);
    struct Node list;
    list.next = NULL;
    for(int i = 0; i < allWords.size; i++) {
        pushUniq(allWords.data[i], allWords.dataItemLen[i], &list);
    }

    struct WordCount work;
    work.taskId = message->type;
    work.allWord = allWords.size;
    work.uniqWordsCount = LIST_SIZE;

    work.uniqWordsLen = calloc(LIST_SIZE, sizeof(int));
    work.uniqWords = calloc(LIST_SIZE, sizeof(char *));

    if (list.next == NULL) {
        response.dataLen = 0;
        response.data = NULL;
        sendMessage(server, &response);
        printf("Empty list\n");

        return NULL;
    }

    struct Node* ptr = list.next;
    int i = 0;
    int size = 0;
    while(ptr->next != NULL) {
        size += strlen(ptr->word);
        work.uniqWords[i] = calloc(strlen(ptr->word), sizeof(char));
        memcpy(work.uniqWords[i], ptr->word, strlen(ptr->word));
        ptr = ptr->next;
    }

    response.dataLen = size * sizeof(char) + (LIST_SIZE + 3) * sizeof(int);
    char* tmp = calloc(response.dataLen, sizeof(char));
    memcpy(tmp, (const void *)&work, response.dataLen);

    response.data = tmp;

    sendMessage(server, &response);

    cleanStringArray(&allWords);
    cleanClientMessage(&response);
}