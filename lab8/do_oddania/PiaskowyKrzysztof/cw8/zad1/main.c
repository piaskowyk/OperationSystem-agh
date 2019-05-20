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
#include <math.h>

#include "utils.h"

int threadCount;
struct IntMatrix photo;
struct FloatMatrix filter;
struct IntMatrix resultImage;

void *runBlockThread(void *data);
void *runInterleavedThread(void *data);
int filterForPixel(int xImageIndex, int yImageIndex);

int main(int argc, char *argv[], char *env[]) {

    int divisionType = 0;
    char* pathToPhoto;
    char* pathToFilter;
    char* pathToResult;

    char* buffer;
    long fileSize;
    struct StringArray fileLines;
    struct StringArray dimension;
    struct StringArray chunks;

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
    dimension = explode(fileLines.data[2], fileSize, ' ');
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
    int widthIndex = 0;
    int heightIndex = 0;

    for(int i = 4; i < fileLines.size; i++){
        if(i > 4) cleanStringArray(&chunks);
        chunks = explode(fileLines.data[i], fileLines.dataItemLen[i], ' ');

        if(chunks.size <= 0) continue;
        for(int k = 0; k < chunks.size; k++) {//at the end of line also is space
            if(widthIndex >= photo.width) {
                widthIndex = 0;
                heightIndex++;
            }

            photo.data[heightIndex * photo.width + widthIndex] = strtol(chunks.data[k], NULL, 0);
            widthIndex++;
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

    widthIndex = 0;
    heightIndex = 0;
    for(int i = 1; i < fileLines.size; i++){
        if(i > 1) cleanStringArray(&chunks);
        chunks = explode(fileLines.data[i], fileLines.dataItemLen[i], ' ');

        if(widthIndex >= filter.width) {
            widthIndex = 0;
            heightIndex++;
        }
        for(int k = 0; k < chunks.size; k++) {
            filter.data[heightIndex * filter.width + widthIndex++] = strtof(chunks.data[k], NULL);
        }
    }

    if(fileLines.size > 1) {
        cleanStringArray(&chunks);
    }
    cleanStringArray(&fileLines);
    free(buffer);
    fclose(fileFilter);

    //test filter matrix
    float sum = 0;
    for(int i = 0; i < filter.height; i++) {
        for(int k = 0; k < filter.width; k++) {
            sum += filter.data[i * filter.width + k];
        }
    }
    if(1 - sum > 0.1 || 1 - sum < -0.1) {
        printErrorMessage("Incorrect filter checksum must be 1", 4);
    }

    //creating threads
    struct timespec startTime = getTimestamp();
    pthread_t* threadsID = calloc(threadCount, sizeof(pthread_t));
    int* order = calloc(threadCount, sizeof(int));

    resultImage.width = photo.width;
    resultImage.height = photo.height;
    initIntMatrix(&resultImage);

    for(int i = 0; i < threadCount; i++) {
        order[i] = i + 1;
        if(divisionType == BLOCK) {
            if(pthread_create(&threadsID[i], NULL, runBlockThread, &order[i]) != 0) {
                printErrorMessage("Unable to create thread", 5);
            }
        } else {
            if(pthread_create(&threadsID[i], NULL, runInterleavedThread, &order[i]) != 0) {
                printErrorMessage("Unable to create thread", 5);
            }
        }
    }

    struct timespec** returnValues = calloc(threadCount, sizeof(struct timespec*));
    for(int i = 0; i < threadCount; i ++) {
        pthread_join(threadsID[i], (void**)&returnValues[i]);
        printf("\033[1;32mInfo:\033[0m Thread %d is end, execution time: %ld s %ld ns\n",
                i+1,
                (returnValues[i])->tv_sec,
                (returnValues[i])->tv_nsec
               );
    }

    struct timespec endTime = getTimestamp();
    struct timespec result = calculateTime(startTime, endTime);

    printf("\033[1;32mInfo:\033[0m Image processing execution time: %ld s %ld ns\n",
            result.tv_sec,
            result.tv_nsec
            );

    free(threadsID);
    free(order);

    //save result image
    FILE* fileResult = fopen(pathToResult, "w");
    if(!fileResult) {
        printErrorMessage("Unable to create result file", 6);
    }

    int breaker = 0;
    fprintf(fileResult, "P2\n%d %d\n255\n", resultImage.width, resultImage.height);
    for(int i = 0; i < resultImage.height; i++) {
        for(int k = 0; k < resultImage.width; k++) {
            fprintf(fileResult, "%d ", resultImage.data[i * resultImage.width + k]);
        }
        breaker++;
        if(breaker > 20) {
            fprintf(fileResult, "\n");
        }
    }
    fclose(fileResult);

    //save rapport files with times
    FILE* fileRapport = fopen(TIMES_RESULTS, "a+");
    if(!fileResult) {
        printErrorMessage("Unable to create rapport file", 6);
    }

    fprintf(fileRapport,
            "Thread count: %d, Image size: %d x %d, type: %d\n",
            threadCount,
            photo.width,
            photo.height,
            divisionType
    );

    for(int i = 0; i < threadCount; i ++) {
        fprintf(fileRapport,
                "Thread %d is end, execution time: %ld s %ld ns\n",
                i+1,
                returnValues[i]->tv_sec,
                returnValues[i]->tv_nsec
        );
    }

    fprintf(fileRapport,
            "Image processing execution time: %ld s %ld ns\n\n",
            result.tv_sec,
            result.tv_nsec
    );

    fclose(fileRapport);

    //clean memory
    cleanIntMatrix(&resultImage);
    cleanIntMatrix(&photo);
    cleanFloatMatrix(&filter);

    printf("\033[1;32mInfo:\033[0m END\n");

    exit(0);
}

void *runBlockThread(void *data) {
    struct timespec startTime = getTimestamp();
    int *k = (int *) data;

    int startIndex = (int)(((*k) - 1) * ceil(resultImage.width / threadCount));
    int endIndex = (int)((*k) * ceil(resultImage.width / threadCount) - 1);

    for(int i = 0; i < resultImage.height; i++) {
        for(int j = startIndex; j <= endIndex; j++) {
            resultImage.data[i * resultImage.width + j] = filterForPixel(j, i);

            if(resultImage.data[i * resultImage.width + j] < 0) {
                resultImage.data[i * resultImage.width + j] = 0;
            }
            if(resultImage.data[i * resultImage.width + j] > 255) {
                resultImage.data[i * resultImage.width + j] = 255;
            }
        }
    }

    struct timespec endTime = getTimestamp();
    struct timespec *result = calloc(1, sizeof(struct timespec));
    *result = calculateTime(startTime, endTime);

    return (void*) result;
}

void *runInterleavedThread(void *data) {
    struct timespec startTime = getTimestamp();
    int *k = (int *) data;

    for(int x = ((*k) - 1); x < resultImage.width; x += threadCount) {
        for(int y = 0; y < resultImage.height; y++) {
            resultImage.data[y * resultImage.width + x] = filterForPixel(x, y);

            if(resultImage.data[y * resultImage.width + x] < 0) {
                resultImage.data[y * resultImage.width + x] = 0;
            }
            if(resultImage.data[y * resultImage.width + x] > 255) {
                resultImage.data[y * resultImage.width + x] = 255;
            }
        }
    }

    struct timespec endTime = getTimestamp();
    struct timespec *result = calloc(1, sizeof(struct timespec));
    *result = calculateTime(startTime, endTime);
    return (void*) result;
}

int max(int a, int b) {
    if(a > b) return a;
    else return b;
}

int filterForPixel(int xImageIndex, int yImageIndex) {
    double result = 0;
    int x, y, c;
    c = filter.width;
    for(int i = 0; i < filter.height; i++) {
        for(int j = 0; j < filter.width; j++) {
            y = max(0, yImageIndex - (int)ceil(c/2) + i - 1);
            x = max(0, xImageIndex - (int)ceil(c/2) + j - 1);

            result += photo.data[y * photo.width + x] * filter.data[i * filter.width + j];
        }
    }

    return (int)round(result);
}