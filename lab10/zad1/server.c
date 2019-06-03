#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#include "utils.h"

#define MAX_CLIENTS 5

unsigned int port = 0;
char* localSocketName;
int running = 1;
struct Client clients[MAX_CLIENTS];
unsigned int clientCount = 0;

pthread_mutex_t clientListMutex = PTHREAD_MUTEX_INITIALIZER;

struct ClientMessage getMessage(int socket);
void sendMessage(int socket, const struct ServerMessage * message);

void handleSIGINT();

void *threadPing(void * data);
void *threadInput(void * data);
void handleRequest(int socket, struct ClientMessage * message);

struct ServerMessage registerAction(struct ClientMessage * message);
struct ServerMessage workDoneAction(struct ClientMessage * message);
struct ServerMessage logoutAction(struct ClientMessage * message);


int main(int argc, char *argv[], char *env[])
{
    //debug
    unlink("../zad1/mleko.tmp");
    //end

    //signal handler init
    struct sigaction actionStruct;
    actionStruct.sa_handler = handleSIGINT;
    sigemptyset(&actionStruct.sa_mask);
    sigaddset(&actionStruct.sa_mask, SIGINT);
    actionStruct.sa_flags = 0;
    sigaction(SIGINT, &actionStruct, NULL);

    for(int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].free = 0;
        clients[i].name = NULL;
    }

    int socketInternetFd, socketLocalFd;
    struct sockaddr_in serverInternetConf;
    struct sockaddr_un serverLocalConf;

    if(argc < 3){
        printErrorMessage("Not enough arguments", 1);
    }

    port = strtol(argv[1], NULL, 0);
    if(port < 1024 || port > 65000) {
        printErrorMessage("Port number must by between 1024 - 65000", 1);
    }

    localSocketName = argv[2];
    if(strcmp(localSocketName, "") == 0){
        printErrorMessage("Socket name can not by empty", 1);
    }

    //create internet socket connection
    socketInternetFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketInternetFd == -1) {
        printErrorMessage("Internet socket creation failed", 2);
    }
    else {
        printf("Create internet socket\n");
    }

    //clear struct
    memset(&serverInternetConf, 0, sizeof(struct sockaddr_in));

    serverInternetConf.sin_family = AF_INET;
    serverInternetConf.sin_addr.s_addr = htonl(INADDR_ANY);
    serverInternetConf.sin_port = htons(port);

    if ((bind(socketInternetFd, (struct sockaddr *)&serverInternetConf, sizeof(serverInternetConf))) != 0) {
        printErrorMessage("Internet socket bind failed", 2);
    }
    else {
        printf("Internet socket successfully bind\n");
    }

    if ((listen(socketInternetFd, MAX_CLIENTS)) != 0) {
        printErrorMessage("Internet socket bind failed", 2);
    }
    else{
        printf("Internet socket listening\n");
    }

    //create local socket connection
    socketLocalFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketLocalFd == -1) {
        printErrorMessage("Local socket creation failed", 3);
    }
    else {
        printf("Create local socket\n");
    }

    memset(&serverLocalConf, 0, sizeof(serverLocalConf));
    serverLocalConf.sun_family = AF_UNIX;
    strcpy(serverLocalConf.sun_path, localSocketName);

    if ((bind(socketLocalFd, (struct sockaddr *)&serverLocalConf, sizeof(serverLocalConf))) != 0) {
        printErrorMessage("Local socket bind failed", 3);
    }
    else {
        printf("Local socket successfully bind\n");
    }

    if ((listen(socketLocalFd, MAX_CLIENTS)) != 0) {
        printErrorMessage("Local socket bind failed", 3);
    }
    else{
        printf("Local socket listening\n");
    }

    struct epoll_event event, events[MAX_CLIENTS];
    int epollFd = epoll_create1(0);
    if(epollFd == -1) {
        printErrorMessage("Epoll failed", 3);
    }
    int event_count;

    if(epollFd == -1) {
        printErrorMessage("Failed to create epoll file descriptor", 4);
    }

    event.events = EPOLLIN | EPOLLET;

    event.data.fd = socketInternetFd;
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, socketInternetFd, &event) == -1) {
        printErrorMessage("Failed to add file descriptor to epoll", 4);
    }

    event.data.fd = socketLocalFd;
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, socketLocalFd, &event) == -1) {
        printErrorMessage("Failed to add file descriptor to epoll", 4);
    }

    //zrobić dwa wątki jeden do pingowania a drugi do wporadzania komend
    //główny wątek służy to odbierania danych

    pthread_t inputThread, pingThread;

    //input thread
    if(pthread_create(&inputThread, NULL, threadInput, NULL) != 0) {
        printErrorMessage("Unable to create input thread", 6);
    }

    if(pthread_detach(inputThread) != 0) {
        printErrorMessage("Unable to detach input thread", 6);
    }

    //ping thread
    if(pthread_create(&pingThread, NULL, threadPing, NULL) != 0) {
        printErrorMessage("Unable to create ping thread", 6);
    }

    if(pthread_detach(pingThread) != 0) {
        printErrorMessage("Unable to detach ping thread", 6);
    }

    //loop to receive data from client
    while(running) {

        printf("Polling for input...\n");
        event_count = epoll_wait(epollFd, events, MAX_CLIENTS, -1);

        for (int i = 0; i < event_count; i++) {

            if (events[i].data.fd == socketLocalFd || events[i].data.fd == socketInternetFd) {
                printf("OK register\n");
                int conn_sock = accept(events[i].data.fd, NULL, NULL);
                if (conn_sock == -1) {
                    printErrorMessage("Error while accept connection", 5);
                }
//                setnonblocking(conn_sock);
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = conn_sock;
                if (epoll_ctl(epollFd, EPOLL_CTL_ADD, conn_sock, &event) == -1) {
                    printErrorMessage("Error epoll_ctl", 5);
                }

            } else {
                struct ClientMessage message = getMessage(events[i].data.fd);
                handleRequest(events[i].data.fd, &message);
                cleanClientMessage(&message);
            }

        }
    }

    pthread_mutex_destroy(&clientListMutex);
    close(socketInternetFd);
    close(socketLocalFd);
    unlink(localSocketName);
}

struct ClientMessage getMessage(int socket) {
    struct ClientMessage message;

    if(read(socket, &message.type, sizeof(message.type)) != sizeof(message.type)){
        printf("Error while reading data\n");
    }
    if(read(socket, &message.dataLen, sizeof(message.dataLen)) != sizeof(message.dataLen)){
        printf("Error while reading data\n");
    }
    if(read(socket, &message.clientNameLen, sizeof(message.clientNameLen)) != sizeof(message.clientNameLen)){
        printf("Error while reading data\n");
    }

    if(message.dataLen > 0) {
        message.data = calloc(message.dataLen + 1, sizeof(char));
        if(message.data == NULL){
            printErrorMessage("Unable to allocate memory", 5);
        }
        if(read(socket, message.data, message.dataLen) != message.dataLen) {
            printf("Error while reading data\n");
        }
    }
    else {
        message.data = NULL;
    }

    if(message.clientNameLen > 0) {
        message.clientName = calloc(message.clientNameLen + 1, sizeof(char));
        if(message.clientName == NULL){
            printErrorMessage("Unable to allocate memory", 5);
        }
        if(read(socket, message.clientName, message.clientNameLen) != message.clientNameLen) {
            printf("Error while reading data\n");
        }
        printf("pp:%s\n",message.clientName);
    }
    else {
        message.clientName = NULL;
    }

    return message;
}

void sendMessage(int socket, const struct ServerMessage *message) {
    write(socket, &message->code, sizeof(message->code));
    write(socket, &message->type, sizeof(message->type));
    write(socket, &message->dataLen, sizeof(message->dataLen));
    write(socket, &message->dataLen, sizeof(message->dataLen));
    if(message->dataLen > 0) {
        write(socket, message->data, message->dataLen * sizeof(char));
    }
}

void handleSIGINT() {
    printf("Receive signal SIGINT.\n");
    running = 0;
}

void *threadPing(void *data) {
    //TODO
}

void *threadInput(void *data) {
    //TODO
}

void handleRequest(int socket, struct ClientMessage * message){

    printf("t: %d\n", message->type);
    printf("nameLn: %d\n", message->clientNameLen);
    printf("name: %s\n", message->clientName);
    printf("dl: %d\n", message->dataLen);
    printf("d: %s\n", message->data);

    struct ServerMessage response;

    switch(message->type){
        case REGISTER_ACTION: {
            response = registerAction(message);
        }
        break;
        case WORK_DONE_ACTION: {
            response = workDoneAction(message);
        }
        break;
        case LOGOUT_ACTION: {
            response = logoutAction(message);
        }
        break;
        default:
            return;
    }

    sendMessage(socket, &response);
}

struct ServerMessage registerAction(struct ClientMessage * message) {
    struct ServerMessage response;
    response.type = REGISTER_ACTION;
    response.dataLen = 0;
    response.data = NULL;

    if(message->clientNameLen <= 0) {
        response.code = INVALID_USER_NAME_T;

        return response;
    }

    pthread_mutex_lock(&clientListMutex);

    //check exist client
    int exist = 0;
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].name == message->clientName) {
            exist = 1;
            break;
        }
    }

    if(exist) {
        pthread_mutex_unlock(&clientListMutex);

        response.code = USER_ALREADY_EXIST_T;

        return response;
    }

    clients[clientCount].free = 0;
    clients[clientCount].name = calloc(message->clientNameLen + 1, sizeof(char));
    memcpy(clients[clientCount].name, message->clientName, message->clientNameLen);
    clientCount++;

    pthread_mutex_unlock(&clientListMutex);

    response.code = USER_ADDED_T;

    return response;
}

struct ServerMessage workDoneAction(struct ClientMessage * message) {
    //TODO
}

struct ServerMessage logoutAction(struct ClientMessage * message) {
    //TODO
}