#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <time.h>

#include "utils.h"

void printErrorMessage(const char * message, int type) {
    fprintf(stderr, "\033[1;32mError:\033[0m %s. errno: %d\n", message, errno);
    perror("Info: ");
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
    for(long i = 0; i < len; i++){
        if(string[i] == delimer) {
            itemsCount++;
        }
    }

    items = calloc(itemsCount, sizeof(char*));
    itemsLen = calloc(itemsCount, sizeof(int));

    int indexGlob, indexStart;
    indexGlob = indexStart = 0;
    for(int i = 0; i < itemsCount; i++) {
        indexStart = indexGlob;
        while(indexGlob < len && string[indexGlob] != delimer) indexGlob++;

        if(indexGlob == indexStart){
            itemsCount--;
            i--;
            continue;
        }
        items[i] = calloc(indexGlob - indexStart + 1, sizeof(char));
        itemsLen[i] = indexGlob - indexStart;
        memcpy(items[i], string + indexStart, (indexGlob - indexStart) * sizeof(char));
        //move after space
        indexGlob++;
    }

    itemsArray.size = itemsCount;
    itemsArray.data = items;
    itemsArray.dataItemLen = itemsLen;

    return itemsArray;
}

void cleanStringArray(struct StringArray * items) {
    for(int i = 0; i < (*items).size; i++) {
        free((*items).data[i]);
    }
    free((*items).data);
}

void initIntMatrix(struct IntMatrix * matrix) {
    (*matrix).data = calloc((*matrix).height, sizeof(int*));
    for(int i = 0; i < (*matrix).height; i++) {
        (*matrix).data[i] = calloc((*matrix).width, sizeof(int));
    }
}

void initFloatMatrix(struct FloatMatrix * matrix) {
    (*matrix).data = calloc((*matrix).height, sizeof(float*));
    for(int i = 0; i < (*matrix).height; i++) {
        (*matrix).data[i] = calloc((*matrix).width, sizeof(float));
    }
}

void cleanIntMatrix(struct IntMatrix * matrix) {
    for(int i = 0; i < (*matrix).height; i++) {
        free((*matrix).data[i]);
    }
    free((*matrix).data);
}

void cleanFloatMatrix(struct FloatMatrix * matrix) {
    for(int i = 0; i < (*matrix).height; i++) {
        free((*matrix).data[i]);
    }
    free((*matrix).data);
}

long getTimestamp() {
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    return timestamp.tv_nsec;
}