#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>

#include "utils.h"

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/un.h>

#define MAX_CLIENTS 5
#define READ_SIZE 10

unsigned int port = 0;
char* localSocketName;
int running = 1;

int main(int argc, char *argv[], char *env[])
{
    //debug
//    close(socketInternetFd);
//    close(socketLocalFd);
    unlink("mleko.tmp");
    //end

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
    int event_count;

    if(epollFd == -1) {
        printErrorMessage("Failed to create epoll file descriptor", 4);
    }

    event.events = EPOLLIN | EPOLLPRI;

    event.data.fd = -socketInternetFd;
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, socketInternetFd, &event) == -1) {
        printErrorMessage("Failed to add file descriptor to epoll", 4);
    }

    event.data.fd = -socketLocalFd;
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, socketLocalFd, &event) == -1) {
        printErrorMessage("Failed to add file descriptor to epoll", 4);
    }

    char read_buffer[READ_SIZE + 1];
    size_t bytes_read;
    while(running)
    {
        printf("\nPolling for input...\n");
        event_count = epoll_wait(epollFd, events, MAX_CLIENTS, 30000);
        printf("%d ready events\n", event_count);
        for(int i = 0; i < event_count; i++)
        {
            printf("Reading file descriptor '%d' -- ", events[i].data.fd);
            bytes_read = read(events[i].data.fd, read_buffer, READ_SIZE);
            printf("%zd bytes read.\n", bytes_read);
            read_buffer[bytes_read] = '\0';
            printf("Read '%s'\n", read_buffer);

            if(!strncmp(read_buffer, "stop\n", 5))
                running = 0;
        }
    }




    close(socketInternetFd);
    close(socketLocalFd);
    unlink(localSocketName);
}