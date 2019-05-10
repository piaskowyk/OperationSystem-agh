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

#include "common.h"

//TODO:
/*
trzeba okodzić to pobieranie z kolejki, wypisywanie tych komunikatów, i sygnał killa obłużyć

bot o ma być taśma czyli co iterację pentli trzeba przesówać wszystkie paczki

trzeba te parametry wejściowe do pamięci współdzielonej zapisać

może się okazać że nie muszę przechowywać ile jest wolnych miejsc bo na początku taśmy będę ustawiać 0 a 0 będzie oznaczać puste miejsce
*/

struct Line {
    unsigned int freePlaceCount;
    unsigned int freeWeightCount;
    unsigned int nextFreeIndex;
};

union semun  
{
    int val;
    struct semid_ds* buf;
    ushort array [1];
} semSetter;

void handleSIGINT(int signalNumber);
void throwError(const char * message, int code);

int semLineState;
struct Line lineParamLocal;

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
    struct ShareMemory lineSM, truckerArgSM, lineParamSM;

    //create shared memory for line
    lineSM.mem = setUpShareMemory(MEM_LINE, lineItemsCapacity * sizeof(unsigned int), IPC_CREAT | STANDARD_PERMISSIONS, 1);

    //create shared memory for tracker args
    truckerArgSM.mem = setUpShareMemory(MEM_TRUCKER_ARG, 3 * sizeof(unsigned int), IPC_CREAT | READ_ONLY_FOR_OTHER, 2);

    //create shared memory for tracker args
    lineParamSM.mem = setUpShareMemory(MEM_LINE_PARAM, 4 * sizeof(unsigned int), IPC_CREAT | STANDARD_PERMISSIONS, 3);

    //create semaphores
    // semaphore for first free index in line
    lineSM.sem = setUpSemaphore(SEM_LINE, 1, 1);

    // count of free places on line
    truckerArgSM.sem = setUpSemaphore(SEM_TRUCKER_ARG, 1, 2);

    // count of free weight on line
    lineParamSM.sem = setUpSemaphore(SEM_LINE_PARAM, 1, 3);

    //set up parameters
    int truckPlacesCount = 0;
    lineParamLocal.freePlaceCount = lineItemsCapacity;
    lineParamLocal.nextFreeIndex = 0;
    lineParamLocal.freeWeightCount = lineWeightCapacity;

    unsigned int* line;
    int endOfLine = lineItemsCapacity - 1;
    int lineLen = lineItemsCapacity;

    //start main loop
    while (1) {
        blockLine();
        //move line
        //get parcel
        //print info
        //update parameters
        
        line = (unsigned int *) shmat(lineSM.mem, 0, 0);
        moveLine(line, lineLen);
        if(lin[endOfLine] > 0) {
            truckPlacesCount += lin[endOfLine];
        }
        else {
            printf("\033[1;32mTrucker:\033[0m Waiting for parcel.\n");
        }

        relaseLine();

        if(truckPlacesCount == truckCapacity) {
            printf("\033[1;32mTrucker:\033[0m Truck is fully - leave factory.\n");
            truckPlacesCount = 0;
            printf("\033[1;32mTrucker:\033[0m New truck arrived to factory.\n");
        }
    }
    
    printf("\033[1;32mTrucker:\033[0m Close.\n");
    
    return 0;
}

void handleSIGINT(int signalNumber) {
    printf("\033[1;32mTrucker:\033[0m Receive signal SIGINT.\n");
}

void throwError(const char * message, int code) {
    fprintf(stderr, "\033[1;32mTrucker:\033[0m %s.\n", message);
    exit(code);
}

void moveLine(unsigned int* line, int len) {
    for(int i = 1; i < len; i ++) {
        line[i] = line[i - 1];
    }
    line[0];
}