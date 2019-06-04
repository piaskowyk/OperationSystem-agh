#pragma once

#include <stdlib.h>
#include <string.h>

struct Node{
    char* word;
    int count;
    struct Node* next;
};

int LIST_SIZE = 0;

void pushUniq(char * word, int len, struct Node* list) {
    if(word == NULL) {
        return;
    }

    struct Node* ptr = list;
    if(list->next != NULL) {
        ptr = list->next;
        while(ptr->next != NULL) {
            if(strcmp(word, ptr->word) == 0){
                ptr->count++;
                return;
            }
            ptr = ptr->next;
        }
    }

    struct Node* item = calloc(1, sizeof(struct Node));
    item->count = 1;
    item->word = calloc(len, sizeof(char));
    memcpy(item->word, word, len);
    item->next = NULL;
    ptr->next = item;
    LIST_SIZE++;
}