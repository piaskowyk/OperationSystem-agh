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
void moveLine(struct Parcel* line, int len);

int main(int argc, char *argv[], char *env[]) {

    //parsing input arguments
    if(argc != 4) {
        printf("\033[1;32mTrucker:\033[0m Invalid count of input arguments.\n");
        exit(100);
    }
   
    unsigned int truckCapacity = strtol(argv[1], NULL, 0);
    unsigned int lineItemsCapacity = strtol(argv[2], NULL, 0);
    unsigned int lineWeightCapacity = strtol(argv[3], NULL, 0);

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
    struct ShareMemory lineSM, lineParamSM;

    //create shared memory for line
    lineSM.mem = setUpShareMemory(MEM_LINE, lineItemsCapacity * sizeof(struct Parcel), 1);

    //create shared memory for tracker args
    lineParamSM.mem = setUpShareMemory(MEM_LINE_PARAM, 10 * sizeof(unsigned int), 2);
    //create semaphores
    // semaphore for first free index in line
    lineSM.sem = setUpSemaphore(SEM_LINE, 1, 1);//albo dać domyślną wartość 0

    // count of free weight on line
    lineParamSM.sem = setUpSemaphore(SEM_LINE_PARAM, 1, 2);// albo dać domyślną wartość 0

    //save line parameters to shared memory
    setFreeWeightOnLine(lineWeightCapacity, lineParamSM);
    blockSem(lineParamSM.sem);
    //clearLine
    blockSem(lineSM.sem);
    struct Parcel* lineClear = (struct Parcel*) shmat(lineSM.mem, 0, 0);
    for(int i = 0; i < lineItemsCapacity; i++) {
        lineClear[i].timestamp = 0;
        lineClear[i].weight = 0;
        lineClear[i].workerId = 0;
    }
    releaseSem(lineSM.sem);

    //set up parameters
    int truckPlacesCount = 0;
    int lineFreeWeightOnLine = lineWeightCapacity;
    struct Parcel* line;
    int endOfLine = lineItemsCapacity - 1;
    int lineLen = lineItemsCapacity;
    //start main loop
    while (1) {
        blockSem(lineSM.sem);

        line = (struct Parcel*) shmat(lineSM.mem, NULL, 0);
        if(line == (void *)-1) {
            fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while attache memory.\n");
            exit(140);
        }

        moveLine(line, lineLen);
        if(line[endOfLine].weight > 0) {
            truckPlacesCount++;
            lineFreeWeightOnLine -= line[endOfLine].weight;
            setFreeWeightOnLine(lineFreeWeightOnLine, lineParamSM);

            printf("\033[1;32mTrucker:\033[0m %ld, Get parcel: weight: %u, workerID: %u, timeDiff: %ld \n",
                    getTimestamp(),
                    line[endOfLine].weight,
                    line[endOfLine].workerId,
                    getTimestamp() - line[endOfLine].timestamp
                );
        }
        else {
            printf("\033[1;32mTrucker:\033[0m %ld, Waiting for parcel.\n", getTimestamp());
        }

        if (shmdt(line) == -1) {
            fprintf(stderr, "\033[1;32mTrucker:\033[0m %ld, Error while detach memory.\n", getTimestamp());
            exit(141);
        }

        releaseSem(lineSM.sem);

        if(truckPlacesCount == truckCapacity) {
            printf("\033[1;32mTrucker:\033[0m %ld, Truck is fully - leave factory.\n", getTimestamp());
            truckPlacesCount = 0;
            printf("\033[1;32mTrucker:\033[0m %ld, New truck arrived to factory.\n", getTimestamp());
        }

        if(endWork) {
            break;
        }
    }

    //empty line
    blockSem(lineSM.sem);
//    blockSem(lineParamSM.sem);
//    releaseSem(lineParamSM.sem);
    line = (struct Parcel*) shmat(lineSM.mem, NULL, 0);
    if(line == (void *)-1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while attache memory.\n");
        exit(140);
    }

    for(int i = 0; i < lineLen; i++) {

        for(int j = 1; j < lineLen; j ++) {
            line[j] = line[j - 1];
        }

        if(line[endOfLine].weight > 0) {
            printf("\033[1;32mTrucker:\033[0m %ld, Get parcel: weight: %u, workerID: %u, timeDiff: %ld \n",
                   getTimestamp(),
                   line[endOfLine].weight,
                   line[endOfLine].workerId,
                   getTimestamp() - line[endOfLine].timestamp
            );
        }
        else {
            printf("\033[1;32mTrucker:\033[0m %ld, Waiting for parcel.\n", getTimestamp());
        }

        if(truckPlacesCount == truckCapacity) {
            printf("\033[1;32mTrucker:\033[0m %ld, Truck is fully - leave factory.\n", getTimestamp());
            truckPlacesCount = 0;
            printf("\033[1;32mTrucker:\033[0m %ld, New truck arrived to factory.\n", getTimestamp());
        }
    }


    if (semctl(lineSM.sem, 0, IPC_RMID) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while removing semaphore (1).\n");
        exit(114);
    }

    if (semctl(lineParamSM.sem, 0, IPC_RMID) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while removing semaphore (2).\n");
        exit(114);
    }

    if (shmctl(lineSM.mem, 0, IPC_RMID) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while removing shared memory (1).\n");
        exit(114);
    }

    if (shmctl(lineParamSM.mem, 0, IPC_RMID) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while removing semaphore (2).\n");
        exit(114);
    }

    printf("\033[1;32mTrucker:\033[0m %ld, END.\n", getTimestamp());

    return 0;
}

void handleSIGINT() {
    printf("\033[1;32mTrucker:\033[0m Receive signal SIGINT.\n");
    endWork = 1;
}

void moveLine(struct Parcel* line, int len) {
    for(int i = 1; i < len; i ++) {
        line[i] = line[i - 1];
    }
    line[0].workerId = 0;
    line[0].weight = 0;
    line[0].timestamp = 0;
}