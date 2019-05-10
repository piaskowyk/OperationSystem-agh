#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <libgen.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include "server_const.h"

void handleSIGINT(int signalNumber);

int main(int argc, char *argv[], char *env[]) {

    //parsing input arguments
    if(argc != 4) {
        printf("\033[1;32mTrucker:\033[0m Invalid count of input arguments.\n");
    }
   
    unsigned int truckCapacity = strtol(argv[1], NULL, 0);
    unsigned int lineItemsCapacity = strtol(argv[2], NULL, 0);
    unsigned int lineWeightCapacity = strtol(argv[3], NULL, 0);

    if(truckCapacity < 1 || lineItemsCapacity < 1 || lineWeightCapacity < 1) {
        printf("\033[1;32mTrucker:\033[0m Invalid input arguments, must be greather than 0.\n");
    }

    //signal handler init
    struct sigaction actionStruct;
    actionStruct.sa_handler = handleSIGINT;
    sigemptyset(&actionStruct.sa_mask); 
    sigaddset(&actionStruct.sa_mask, SIGINT); 
    actionStruct.sa_flags = 0;
    sigaction(SIGINT, &actionStruct, NULL); 
    
    //create line and counters


    printf("\033[1;32mTrucker:\033[0m Close.\n");
    
    return 0;
}

void handleSIGINT(int signalNumber) {
    printf("\033[1;32mTrucker:\033[0m Receive signal SIGINT.\n");
}