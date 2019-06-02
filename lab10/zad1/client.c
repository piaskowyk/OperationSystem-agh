#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>

#include "utils.h"

#define MAX_NAME_LEN 10
#define LOCAL_CONNECTION 1
#define INTERNET_CONNECTION 2

char* clientName;
int connectionType;
char* connectionAddress;

int main(int argc, char *argv[], char *env[]) {

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




    close(socketFd);
}