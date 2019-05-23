#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "utils.h"

int passengerCount;
int carriageCount;
int carriageCapacity;
int tourCount;

void *threadCarriage(void *data);
void *threadPassenger(void *data);

int main(int argc, char *argv[], char *env[]) {

    if(argc < 5){
        printErrorMessage("Not enough arguments", 1);
    }

    passengerCount = strtol(argv[1], NULL, 0);
    if(passengerCount < 1) {
        printErrorMessage("Passenger count must by greater than 0", 1);
    }

    carriageCount = strtol(argv[2], NULL, 0);
    if(carriageCount < 1) {
        printErrorMessage("Carriage count must by greater than 0", 1);
    }

    carriageCapacity = strtol(argv[3], NULL, 0);
    if(carriageCapacity < 1) {
        printErrorMessage("Carriage capacity must by greater than 0", 1);
    }

    tourCount = strtol(argv[4], NULL, 0);
    if(tourCount < 1) {
        printErrorMessage("Tour count must by greater than 0", 1);
    }

    if(passengerCount < carriageCount * carriageCapacity) {
        printErrorMessage("Too enough passenger", 1);
    }

    //creating carriage threads
    pthread_t* carriageKey = calloc(carriageCount, sizeof(pthread_t));
    int* carriageID = calloc(carriageCount, sizeof(int));

    for(int i = 0; i < carriageCount; i++) {
        carriageID[i] = i;
        if(pthread_create(&carriageKey[i], NULL, threadCarriage, &carriageID[i]) != 0) {
            printErrorMessage("Unable to create thread", 2);
        }
    }

    //creating passenger threads
    pthread_t* passengerKey = calloc(passengerCount, sizeof(pthread_t));
    int* passengerID = calloc(passengerCount, sizeof(int));

    for(int i = 0; i < passengerCount; i++) {
        passengerID[i] = i;
        if(pthread_create(&passengerKey[i], NULL, threadPassenger, &passengerID[i]) != 0) {
            printErrorMessage("Unable to create thread", 3);
        }
    }

    //ending carriage threads
    for(int i = 0; i < carriageCount; i ++) {
        pthread_join(carriageKey[i], NULL);
        printf("\033[1;33m>:\033[0m Carriage's thread is end - %d, time: %ld\n",
            i,
            getTimestamp()
        );
    }

    //ending passenger threads
    for(int i = 0; i < passengerCount; i ++) {
        pthread_join(passengerKey[i], NULL);
        printf("\033[1;32m>:\033[0m Passenger's thread is end - %d, time: %ld\n",
               i,
               getTimestamp()
        );
    }

    free(carriageKey);
    free(carriageID);

    free(passengerKey);
    free(passengerID);

    printf("\033[1;34m>:\033[0m END\n");

    return 0;
}

void *threadCarriage(void *data) {
    printf("\033[1;33m[%ld]>:\033[0m \n", getTimestamp());
    return NULL;
}

void *threadPassenger(void *data) {
    printf("\033[1;32m[%ld]>:\033[0m \n", getTimestamp());
    return NULL;
}