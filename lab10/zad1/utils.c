#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

void printErrorMessage(const char * message, int type) {
    fprintf(stderr, "\033[1;34mError:\033[0m %s. errno: %d\n", message, errno);
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