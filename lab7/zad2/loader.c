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
#include <sys/mman.h>
#include <semaphore.h> 

#include "common.h"

int endWork = 0;

void handleSIGINT();

int main(int argc, char *argv[], char *env[]) {

    int parcelWeight = 0;
    int parcelCount = 0;
    int workersCount = 0;
    struct LineParams* lineParams;
    struct Parcel* line;
    int lineParamsSize;
    int lineSize;

    //parsing input arguments
    if(argc > 2) {
        workersCount = strtol(argv[1], NULL, 0);
        parcelWeight = strtol(argv[2], NULL, 0);
    }

    if (argc > 3) {
        parcelCount = strtol(argv[3], NULL, 0);
    }

    if(parcelWeight < 1 || (argc > 3 && parcelCount < 1)) {
        printf("\033[1;33mLoader:\033[0m Invalid of input arguments.\n");
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
    int lineSM, lineParamSM;
    sem_t* semaphore;

    semaphore = setUpSemaphore(SEM, LOADER);

    lineParamsSize = sizeof(struct LineParams);

    lineParamSM = setUpShareMemory(MEM_LINE_PARAM, lineParamsSize, 1, LOADER);

    lineParams = getLineParams(lineParamSM, lineParamsSize, LOADER);

    lineSize = lineParams[0].len * sizeof(struct Parcel);
    
    lineParams[0].loadersCount = workersCount;
    lineParams[0].loadersCountEndWord = 0;

    lineSM = setUpShareMemory(MEM_LINE, lineSize, 2, LOADER);
    releaseLineParams(lineParams, lineParamsSize, LOADER);

    for(int i = 0; i < workersCount; i++) {
        pid_t pid = fork();
        if(pid == 0) break;
    }

    //set up parameters
    int iteration = 0;
    pid_t ownPid = getpid();

    //start main loop
    while(!endWork && ((argc > 3 && parcelCount > iteration))) {
        printf("\033[1;33mLoader:\033[0m %ld, Waiting for line. PID: %d\n", getTimestamp(), ownPid);
        blockSem(semaphore, LOADER);
        line = getLine(lineSM, lineSize, LOADER);
        lineParams = getLineParams(lineParamSM, lineParamsSize, LOADER);

        if(lineParams[0].freeWeight >= parcelWeight && lineParams[0].freePlaces > 0 && line[0].weight == 0) {

            //push parcel
            line[0].workerId = ownPid;
            line[0].weight = parcelWeight;
            line[0].timestamp = getTimestamp();

            //set line params
            lineParams[0].freePlaces--;
            lineParams[0].freeWeight -= line[0].weight;


            printf("\033[1;33mLoader:\033[0m %ld, Push new parcel: weight: %u, workerID: %u, time: %ld, freePlaces: %d, freeWeight: %d\n",
                   getTimestamp(),
                   parcelWeight,
                   ownPid,
                   getTimestamp(),
                   lineParams[0].freePlaces,
                   lineParams[0].freeWeight
                   );

            if(argc > 3) iteration++;
        }
        else {
            printf("\033[1;33mLoader:\033[0m %ld, Waiting places on line. PID: %d\n", getTimestamp(), ownPid);
        }
        printf("w:%d,p:%d,l:%d\n", lineParams[0].freeWeight, lineParams[0].freePlaces, lineParams[0].len);
        releaseLine(line, lineSize, LOADER);
        releaseLineParams(lineParams, lineParamsSize, LOADER);
        releaseSem(semaphore, LOADER);

        if(DEBUG) sleep(1);
        if(lineParams[0].mode) break;
    }

    //zamknięcie semaforów
    if(sem_close(semaphore) == -1) {
        printErrorMessage("Error while closing semaphore", LOADER);
        exit(130);
    }

    printf("\033[1;33mLoader:\033[0m %ld, END.\n", getTimestamp());

    return 0;
}

void handleSIGINT() {
    printf("\033[1;33mLoader:\033[0m Receive signal SIGINT.\n");
    endWork = 1;
}