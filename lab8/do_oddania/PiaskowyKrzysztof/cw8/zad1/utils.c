#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <time.h>

#include "utils.h"

void printErrorMessage(const char * message, int type) {
    fprintf(stderr, "\033[1;32mError:\033[0m %s. errno: %d\n", message, errno);
    if(errno > 0) {
        perror("Errno info");
    }
    if(type > 0){
        exit(type);
    }
}

struct StringArray explode(char* string, long len, char delimer) {
    struct StringArray itemsArray;
    char** items = NULL;
    int* itemsLen;
    int itemsCount = 0;

    itemsArray.size = 0;
    itemsArray.data = NULL;
    itemsArray.dataItemLen = NULL;

    if(len == 0 || string == NULL) return itemsArray;

    itemsCount++;
    for(long i = 0; i < len; i++) {
        if(string[i] == delimer) {
            itemsCount++;
        }
    }

    items = calloc(itemsCount, sizeof(char*));
    itemsLen = calloc(itemsCount, sizeof(int));

    int indexGlob, indexStart;
    indexGlob = indexStart = 0;
    for(int i = 0; i < itemsCount; i++) {
        while(indexGlob < len && string[indexGlob] == delimer) indexGlob++;//remove many delimers
        indexStart = indexGlob;
        while(indexGlob < len && string[indexGlob] != delimer) indexGlob++;

        if(indexGlob == indexStart) {
            itemsCount--;
            i--;
            continue;
        }

        items[i] = calloc(indexGlob - indexStart + 1, sizeof(char));
        itemsLen[i] = indexGlob - indexStart;
        memcpy(items[i], string + indexStart, (indexGlob - indexStart) * sizeof(char));
    }

    itemsArray.size = itemsCount;
    itemsArray.data = items;
    itemsArray.dataItemLen = itemsLen;

    return itemsArray;
}

void cleanStringArray(struct StringArray * items) {
    if((*items).data == NULL) return;
    for(int i = 0; i < (*items).size; i++) {
        if((*items).data[i] == NULL) continue;
        free((*items).data[i]);
    }
    free((*items).data);
}

void initIntMatrix(struct IntMatrix * matrix) {
    (*matrix).data = calloc((*matrix).height * (*matrix).width, sizeof(int*));
}

void cleanIntMatrix(struct IntMatrix * matrix) {
    if((*matrix).data == NULL) return;
    free((*matrix).data);
}

void initFloatMatrix(struct FloatMatrix * matrix) {
    (*matrix).data = calloc((*matrix).height * (*matrix).width, sizeof(int*));
}

void cleanFloatMatrix(struct FloatMatrix * matrix) {
    if((*matrix).data == NULL) return;
    free((*matrix).data);
}

struct timespec getTimestamp() {
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    return timestamp;
}

struct timespec calculateTime(struct timespec startTime, struct timespec stopTime) {
    struct timespec result;

    if((stopTime.tv_nsec - startTime.tv_nsec) < 0){
        result.tv_sec = stopTime.tv_sec - startTime.tv_sec - 1;
        result.tv_nsec = 1000000000 + stopTime.tv_nsec - startTime.tv_nsec;
    }
    else {
        result.tv_sec = stopTime.tv_sec - startTime.tv_sec;
        result.tv_nsec = stopTime.tv_nsec - startTime.tv_nsec;
    }

    return result;
}