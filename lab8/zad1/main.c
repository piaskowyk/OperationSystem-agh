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

void *print_message_function( void *ptr );

int main(int argc, char *argv[], char *env[]) {

    int threadCount;
    int divisionType = 0;
    char* pathToPhoto;
    char* pathToFilter;
    char* pathToResult;

    char* buffer;
    long fileSize;
    struct StringArray fileLines;
    struct StringArray dimension;
    struct StringArray chunks;
    struct IntMatrix photo;
    struct FloatMatrix filter;

    if(argc < 6){
        printErrorMessage("Not enough arguments", 1);
    }

    threadCount = strtol(argv[1], NULL, 0);
    if(threadCount < 1) {
        printErrorMessage("Thread count must by greater than 0", 1);
    }

    if(strcmp(argv[2], "block") == 0) {
        divisionType = BLOCK;
    }
    else if(strcmp(argv[2], "interleaved") == 0) {
        divisionType = INTERLEAVED;
    }
    else {
        printErrorMessage("Division type is only block or interleaved", 1);
    }

    pathToPhoto = argv[3];
    pathToFilter = argv[4];
    pathToResult = argv[5];

    //load image
    FILE* fileImage = fopen(pathToPhoto, "r");
    if(!fileImage) {
        printErrorMessage("Error while opening image file", 1);
    }

    fseek(fileImage , 0L, SEEK_END);
    fileSize = ftell(fileImage);
    rewind(fileImage);

    buffer = calloc(fileSize, sizeof(char));
    fread(buffer, fileSize, sizeof(char), fileImage);

    //get palette range
    fileLines = explode(buffer, fileSize, '\n');
    if(fileLines.size < 5) {
        printErrorMessage("Incorrect image file format", 1);
    }

    int range = strtol(fileLines.data[3], NULL, 0);
    if(range != 255) {
        printErrorMessage("Incorrect color palette range", 1);
    }

    //get dimension of image
    dimension = explode(fileLines.data[2], fileSize, '\n');
    if(dimension.size < 2) {
        printErrorMessage("Incorrect image file format", 2);
    }

    photo.width = strtol(dimension.data[0], NULL, 0);
    photo.height = strtol(dimension.data[1], NULL, 0);
    if(photo.width < 1 || photo.height < 1) {
        printErrorMessage("Incorrect image dimension", 2);
    }

    cleanStringArray(&dimension);

    //reading input raw image date
    initIntMatrix(&photo);
    int weightIndex = 0;
    int heightIndex = 0;

    for(int i = 4; i < fileLines.size; i++){
        if(i > 4) cleanStringArray(&chunks);
        chunks = explode(fileLines.data[i], fileLines.dataItemLen[i], ' ');

        for(int k = 0; k < chunks.size - 1; k++) {//at the end of line also is space
            //if is empty go thought
            if(chunks.data[k][0] == ' ') continue;

            if(weightIndex >= photo.width) {
                weightIndex = 0;
                heightIndex++;
            }
            photo.data[heightIndex][weightIndex++] = strtol(chunks.data[k], NULL, 0);
        }
    }

    fclose(fileImage);

    //load filter data
    FILE* fileFilter = fopen(pathToFilter, "r");
    if(!fileFilter) {
        printErrorMessage("Error while opening filter file", 3);
    }

    fseek(fileImage , 0L, SEEK_END);
    fileSize = ftell(fileImage);
    rewind(fileImage);

    free(buffer);
    buffer = calloc(fileSize, sizeof(char));
    fread(buffer, fileSize, sizeof(char), fileImage);

    //get size od filter matrix
    cleanStringArray(&fileLines);
    fileLines = explode(buffer, fileSize, '\n');
    if(fileLines.size < 2) {
        printErrorMessage("Incorrect filter file format", 3);
    }

    filter.width = strtol(fileLines.data[0], NULL, 0);
    if(filter.width < 1) {
        printErrorMessage("Incorrect filter dimension", 3);
    }
    filter.height = filter.width;
    initFloatMatrix(&filter);

    weightIndex = 0;
    heightIndex = 0;

    for(int i = 1; i < fileLines.size; i++){
        if(i > 1) cleanStringArray(&chunks);
        chunks = explode(fileLines.data[i], fileLines.dataItemLen[i], ' ');

        if(weightIndex >= filter.width) {
            weightIndex = 0;
            heightIndex++;
        }
        for(int k = 0; k < chunks.size; k++) {
            filter.data[heightIndex][weightIndex++] = strtof(chunks.data[k], NULL);
        }
    }

    if(fileLines.size > 1) {
        cleanStringArray(&chunks);
    }
    cleanStringArray(&fileLines);
    cleanStringArray(&dimension);
    free(buffer);
    fclose(fileFilter);

    //test filter matrix
    float sum = 0;
    for(int i = 0; i < filter.height; i++) {
        for(int k = 0; k < filter.width; k++) {
            sum += filter.data[i][k];
        }
    }
    if(1 - sum > 0.1 || 1 - sum < -0.1) {
        printErrorMessage("Incorrect filter checksum must be 1", 4);
    }

    //creating threads

    /*
     * zaczynam mieżyć czas
     * robie sobie odpowiednie funkcje w zależności od typu intervału po jakim mam chodzić
     * wywołuje wszystkie wątki
     * mierzę czas w wątkach
     * robię filtr na zdjęciu
     * pobieram czas wątku
     * wypisuje na ekranie
     * czekam aż się skończą wszystkei wątki
     * wypisuje czas całego programu
     * zapisuje obraz po filtrowaniu do pliku
     * zapisuje uzyskane czasy do pliku
     * */
    for(int i = 0; i < threadCount; i ++) {
        if(divisionType == BLOCK) {
            //blockThread();
        } else {
            //interleavedThread();
        }
    }

////////////////////////////////////////////////////////////////////

    pthread_t thread1, thread2;
    char *message1 = "Thread 1 tmp - ";
    char *message2 = "Thread 2";
    int  iret1, iret2;

    iret1 = pthread_create( &thread1, NULL, print_message_function, (void*) message1);
    iret2 = pthread_create( &thread2, NULL, print_message_function, (void*) message2);

    int returnValue1;
    int returnValue2;
    pthread_join( thread1, (void**)&returnValue1);
    pthread_join( thread2, (void**)&returnValue2);

    printf("Thread 1 returns: %d, %d\n", iret1, returnValue1);
    printf("Thread 2 returns: %d, %d\n", iret2, returnValue2);
    exit(0);
}

void *print_message_function( void *ptr )
{
    char *message;
    message = (char *) ptr;
    printf("%s \n", message);

    if(strcmp(message, "Thread 2") != 0) {
        pthread_exit((void*)4);
    }
    sleep(5);
    return (void*) 8;
}