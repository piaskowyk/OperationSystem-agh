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
#include <sys/sem.h>

#include "common.h"

int endWork = 0;

void handleSIGINT();

int main(int argc, char *argv[], char *env[]) {

    struct LineParams* lineParams;
    struct Parcel* line;

    //parsing input arguments
    if(argc != 4 && argc != 5) {
        printf("\033[1;32mTrucker:\033[0m Invalid count of input arguments.\n");
        exit(100);
    }
   
    unsigned int truckCapacity = strtol(argv[1], NULL, 0);
    unsigned int lineItemsCapacity = strtol(argv[2], NULL, 0);
    unsigned int lineWeightCapacity = strtol(argv[3], NULL, 0);

    unsigned int forceStopParcelCounter = 0;

    if(argc == 5) {
        forceStopParcelCounter = strtol(argv[4], NULL, 0);
    }

    if(truckCapacity < 1 || lineItemsCapacity < 1 || lineWeightCapacity < 1) {
        printf("\033[1;32mTrucker:\033[0m Invalid input arguments, must be greater than 0.\n");
        exit(100);
    }

    //signal handler init
    struct sigaction actionStruct;
    actionStruct.sa_handler = handleSIGINT;
    sigemptyset(&actionStruct.sa_mask);
    sigaddset(&actionStruct.sa_mask, SIGINT);
    actionStruct.sa_flags = 0;
    sigaction(SIGINT, &actionStruct, NULL);

    //create line and counters
    int lineSM, lineParamSM, semaphore;

    //create shared memory for line
    lineSM = setUpShareMemory(MEM_LINE, lineItemsCapacity * sizeof(struct Parcel), 1, TRUCKER);

    //create shared memory for tracker args
    lineParamSM = setUpShareMemory(MEM_LINE_PARAM, sizeof(struct LineParams), 2, TRUCKER);
    
    //create semaphores
    semaphore = setUpSemaphore(SEM, 1, TRUCKER);

    //save line parameters to shared memory
    blockSem(semaphore, TRUCKER);
    lineParams = getLineParams(lineParamSM, TRUCKER);
    lineParams[0].freePlaces = lineItemsCapacity;
    lineParams[0].freeWeight = lineWeightCapacity;
    lineParams[0].len = lineItemsCapacity;
    releaseLineParams(lineParams, TRUCKER);

    //clearLine
    struct Parcel* lineClear = (struct Parcel*) shmat(lineSM, 0, 0);
    for(int i = 0; i < lineItemsCapacity; i++) {
        lineClear[i].timestamp = 0;
        lineClear[i].weight = 0;
        lineClear[i].workerId = 0;
    }
    releaseSem(semaphore, TRUCKER);

    printf("Delay for start 5s\n");
    for(int i = 0; i < 5; i++){
        printf("%d\n", 5 - i);
        sleep(1);
    }
    printf("START");

    //set up parameters
    int truckPlacesCount = 0;
    int endOfLine = lineItemsCapacity - 1;
    int lineLen = lineItemsCapacity;
    int counter = 0;

    //start main loop
    while (1) {
        blockSem(semaphore, TRUCKER);
        line = getLine(lineSM, TRUCKER);
        lineParams = getLineParams(lineParamSM, TRUCKER);

        if(line[endOfLine].weight > 0) {
            truckPlacesCount++;
            
            //set line params
            lineParams[0].freePlaces++;
            lineParams[0].freeWeight += line[endOfLine].weight;

            //get parcel
            line[endOfLine].weight = 0;
            line[endOfLine].timestamp = 0;
            line[endOfLine].workerId = 0;

            printf("\033[1;32mTrucker:\033[0m %ld, Get parcel: weight: %u, workerID: %u, timeDiff: %ld, capacity: %d/%d  \n",
                    getTimestamp(),
                    line[endOfLine].weight,
                    line[endOfLine].workerId,
                    getTimestamp() - line[endOfLine].timestamp,
                    truckPlacesCount,
                    truckCapacity
                );

            counter++;
            if(counter == forceStopParcelCounter){
                printf("\033[1;32mTrucker:\033[0m %ld, Force stop, receive %d parcels.\n", getTimestamp(), counter);
                endWork = 1;
            }
        }
        else {
            printf("\033[1;32mTrucker:\033[0m %ld, Waiting for parcel.\n", getTimestamp());
        }

        moveLine(line, lineLen);

        printf("w:%d,p:%d,l:%d\n", lineParams[0].freeWeight, lineParams[0].freePlaces, lineParams[0].len);
        releaseLine(line, TRUCKER);
        releaseLineParams(lineParams, TRUCKER);
        releaseSem(semaphore, TRUCKER);

        if(truckPlacesCount == truckCapacity) {
            printf("\033[1;32mTrucker:\033[0m %ld, Truck is fully - leave factory.\n", getTimestamp());
            truckPlacesCount = 0;
            printf("\033[1;32mTrucker:\033[0m %ld, New truck arrived to factory.\n", getTimestamp());
        }

        if(endWork) {
            break;
        }

        if(DEBUG) sleep(1);
    }

    //empty line
    blockSem(semaphore, TRUCKER);
    line = getLine(lineSM, TRUCKER);

    for(int i = 0; i < lineLen; i++) {

        moveLine(line, lineLen);

        if(line[endOfLine].weight > 0) {
            printf("\033[1;32mTrucker:\033[0m %ld, Get parcel (end): weight: %u, workerID: %u, timeDiff: %ld \n",
                   getTimestamp(),
                   line[endOfLine].weight,
                   line[endOfLine].workerId,
                   getTimestamp() - line[endOfLine].timestamp
            );
        }
        else {
            printf("\033[1;32mTrucker:\033[0m %ld, Waiting for parcel (end).\n", getTimestamp());
        }

        if(truckPlacesCount == truckCapacity) {
            printf("\033[1;32mTrucker:\033[0m %ld, Truck is fully - leave factory (end).\n", getTimestamp());
            truckPlacesCount = 0;
            printf("\033[1;32mTrucker:\033[0m %ld, New truck arrived to factory (end).\n", getTimestamp());
        }
    }


    if (semctl(semaphore, 0, IPC_RMID) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while removing semaphore.\n");
        exit(114);
    }

    if (shmctl(lineSM, 0, IPC_RMID) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while removing shared memory (1).\n");
        exit(114);
    }

    if (shmctl(lineParamSM, 0, IPC_RMID) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while removing shared memory (2).\n");
        exit(114);
    }

    printf("\033[1;32mTrucker:\033[0m %ld, END.\n", getTimestamp());

    return 0;
}

void handleSIGINT() {
    printf("\033[1;32mTrucker:\033[0m Receive signal SIGINT.\n");
    endWork = 1;
}