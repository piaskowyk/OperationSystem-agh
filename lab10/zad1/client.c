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

#include "utils.h"

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
        printf("Waiting...\n");

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

    if(read(socket, &message.code, sizeof(message.code)) != sizeof(message.code)){
        printf("Error while reading data %d\n", errno);
    }

    if(read(socket, &message.type, sizeof(message.type)) != sizeof(message.type)){
        printf("Error while reading data %d\n", errno);
    }

    if(read(socket, &message.dataLen, sizeof(message.dataLen)) != sizeof(message.dataLen)){
        printf("Error while reading data %d\n", errno);
    }

    if(message.dataLen > 0) {
        message.data = calloc(message.dataLen + 1, sizeof(char));
        if(message.data == NULL){
            printErrorMessage("Unable to allocate memory", 5);
        }
        if(read(socket, message.data, message.dataLen) != message.dataLen) {
            printf("Error while reading data %d\n", errno);
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
            response = workAction(message);
        }
        break;
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
    printf("PING action\n");

    struct ClientMessage response;

    response.clientName = calloc(strlen(clientName), sizeof(char));
    memcpy(response.clientName, clientName, strlen(clientName));
    response.clientNameLen = strlen(clientName);
    response.type = PING_ACTION;
    response.dataLen = 0;
    response.data = NULL;

    return response;
}

struct ClientMessage workAction(struct ServerMessage *message) {
    struct ClientMessage response;

    response.clientName = calloc(strlen(clientName), sizeof(char));
    memcpy(response.clientName, clientName, strlen(clientName));
    response.clientNameLen = strlen(clientName);
    response.type = PING_ACTION;
    response.dataLen = 5;
    response.data = "mleko";

    return response;
}