#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>

struct WatchingItem {
    char* pathToFile;
    unsigned int interval;
};

int watcherCopyViaProg(struct WatchingItem watchingItem, int monitoringTime);
int watcherCopyViaCmd(struct WatchingItem watchingItem, int monitoringTime);
void clean(struct WatchingItem* watchingItem,  int howItem, pid_t* process);

int main(int argc, char *argv[]) {
    int validInput = 1;

    char* listFileName = "";
    unsigned int monitoringTime = 0;
    int mode = 0;
    char *end;

    if(argc < 4){
        fputs("Not enough input arguments.\n", stderr);
        exit(100);
    }

    if(argc == 4){
        listFileName = argv[1];
        monitoringTime = (unsigned int)strtol(argv[2], &end, 0);
        mode = (unsigned int)strtol(argv[3], &end, 0);
    }
    else {
        validInput = 0;
    }

    if(!validInput){
        fputs("Invalid input arguments.\n", stderr);
        exit(101);
    }

    FILE* fileList = fopen(listFileName, "r");
    if(!fileList){
        fputs("Error while opening file with list of files to watching.\n", stderr);
        exit(102);
    }

    long fileSize;
    fseek(fileList , 0L, SEEK_END);
    fileSize = ftell(fileList);
    rewind(fileList);

    char* fileListContent = calloc((size_t )fileSize, sizeof(char));
    fread(fileListContent, (size_t )fileSize, sizeof(char), fileList);
    fclose(fileList);

    int fileLinesCount  = 0;
    for(int i = 0; i < fileSize; i++) {
        if(fileListContent[i] == '\n') fileLinesCount++;
    }
    fileLinesCount++;
    if(fileListContent[fileSize - 1] == '\n') fileLinesCount--;
    int howWatchingFile = fileLinesCount;

    //parsowanie pliku lista
    struct WatchingItem* watchingItems = calloc((size_t )fileLinesCount, sizeof(struct WatchingItem));
    int index = 0;
    for(int wathingItemIndex = 0; wathingItemIndex < howWatchingFile; wathingItemIndex++) {
        //parsowanie ścieżki do pliku
        int indexStart = index;
        int len = 0;
        while(index < fileSize && fileListContent[index] != ' '){
            len++;
            index++;
        }
        char* filePath = calloc((size_t )len, sizeof(char));
        for(int k = 0; k < len; k++){
            filePath[k] = fileListContent[indexStart + k];
        }
        watchingItems[wathingItemIndex].pathToFile = filePath;

        //parsowanie interwału
        index++;
        indexStart = index;
        len = 0;
        while(index < fileSize && fileListContent[index] != '\n') {
            len++;
            index++;
        }
        char* intervalStr = calloc((size_t )len, sizeof(char));
        for(int k = 0; k < len; k++){
            intervalStr[k] = fileListContent[indexStart + k];
        }

        watchingItems[wathingItemIndex].interval = (unsigned int)strtol(intervalStr, &end, 0);
        index = indexStart + len + 1;
        free(intervalStr);
    }
    free(fileListContent);

    //tworzenie katalogu archiwum jeśli nie isitnieje
    struct stat st = {0};
    if (stat("./archiwum", &st) == -1) {
        mkdir("./archiwum", 0700);
    }

    //startowanie nasłuchiwania
    pid_t* process = calloc((size_t )howWatchingFile, sizeof(pid_t));
    for(int i = 0; i < howWatchingFile; i++){
        pid_t pid;
        pid = fork();
        if (pid > 0) {
            process[i] = pid;
        }
        else if (pid == 0){
            int result;
            if(mode){
                result = watcherCopyViaProg(watchingItems[i], monitoringTime);
            }
            else {
                result = watcherCopyViaCmd(watchingItems[i], monitoringTime);
            }
            clean(watchingItems, howWatchingFile, process);
            
            return result;
        }
        else {
            fputs("Error while starting process\n", stderr);
        }
    }

    //zbieranie statusów procesów
    for(int i = 0; i < howWatchingFile; i++){
        pid_t wait;
        int status;
        wait = waitpid(process[i], &status, WUNTRACED);
        if(wait == -1){
            fputs("Error while stoping process\n", stderr);
        }

        if(WEXITSTATUS(status) || WEXITSTATUS(status) == 0){
            printf("Process PID:%d copped file %d times\n", process[i], WEXITSTATUS(status));
        }
        else {
            printf("Error process %d, with code: %d\n", wait, WEXITSTATUS(status));
        }
    }
    clean(watchingItems, howWatchingFile, process);

    return 0;
}

int watcherCopyViaProg(struct WatchingItem watchingItem, int monitoringTime){
    FILE* watchingFile = fopen(watchingItem.pathToFile, "r");
    if(!watchingFile){
        printf("Error while opening file: %s\n", watchingItem.pathToFile);
        exit(0);
    }

    long lastFileSize = 0;
    fseek(watchingFile , 0L, SEEK_END);
    lastFileSize = ftell(watchingFile);
    rewind(watchingFile);

    char* lastFileContent = calloc((size_t )lastFileSize, sizeof(char));
    fread(lastFileContent, (size_t )lastFileSize, sizeof(char), watchingFile);
    fclose(watchingFile);

    int howCopies = 0;
    __time_t lastTimeMod = 0;
    char* fileName = basename(watchingItem.pathToFile);
    for(int i = 0; i < monitoringTime / watchingItem.interval; i++){
        struct stat file_info;
        stat(watchingItem.pathToFile, &file_info);
        if(lastTimeMod < file_info.st_mtime){
            printf("Create new backup of file: %s\n", watchingItem.pathToFile);
            howCopies++;
            lastTimeMod = file_info.st_mtime;

            //generowanie nazwy backupu pliku
            __time_t now; 
            time(&now);
            char date[21];
            strftime(date, 21, "_%d-%m-%Y_%H:%M:%S", localtime(&now));
            size_t fileNameLen  = strlen(fileName);
            size_t backupNameLen = fileNameLen + 21;
            char* backupName = calloc(backupNameLen, sizeof(char));
            memcpy(backupName, "./archiwum/", 11);
            memcpy(backupName + 11, fileName, fileNameLen);
            memcpy(backupName + 11 + fileNameLen, date, 21);

            //tworzenie pliku z backupem
            FILE* backupFile = fopen(backupName, "w+");
            if(!backupFile){
                printf("Error while opening file: %s\n", watchingItem.pathToFile);
                exit(howCopies);
            }

            fwrite(lastFileContent, (size_t )lastFileSize, sizeof(char), backupFile);
            fclose(backupFile);
            free(backupName);

            //wczytywanie nowej wersji pliku
            watchingFile = fopen(watchingItem.pathToFile, "r");
            if(!watchingFile){
                printf("Error while opening file: %s\n", watchingItem.pathToFile);
                exit(howCopies);
            }

            fseek(watchingFile , 0L, SEEK_END);
            lastFileSize = ftell(watchingFile);
            rewind(watchingFile);
            free(lastFileContent);

            lastFileContent = calloc((size_t )lastFileSize, sizeof(char));
            fread(lastFileContent, (size_t )lastFileSize, sizeof(char), watchingFile);
            fclose(watchingFile);
        }
        //czekaj do następnego sprawdzenia daty
        sleep(watchingItem.interval);
    }
    free(lastFileContent);

    return howCopies;
}

int watcherCopyViaCmd(struct WatchingItem watchingItem, int monitoringTime){
    int howCopies = -1;
    __time_t lastTimeMod = 0;
    char* fileName = basename(watchingItem.pathToFile);
    for(int i = 0; i < monitoringTime / watchingItem.interval; i++){
        struct stat file_info;
        stat(watchingItem.pathToFile, &file_info);
        if(lastTimeMod < file_info.st_mtime || howCopies == -1){
            printf("Create new backup of file: %s\n", watchingItem.pathToFile);
            howCopies++;
            lastTimeMod = file_info.st_mtime;

            //generowanie nazwy backupu pliku
            __time_t now; 
            time(&now);
            char date[21];
            strftime(date, 21, "_%d-%m-%Y_%H:%M:%S", localtime(&now));
            size_t fileNameLen  = strlen(fileName);
            size_t backupNameLen = fileNameLen + 21;
            char* backupName = calloc(backupNameLen, sizeof(char));
            memcpy(backupName, "./archiwum/", 11);
            memcpy(backupName + 11, fileName, fileNameLen);
            memcpy(backupName + 11 + fileNameLen, date, 21);
            
            pid_t pid;
            pid = fork();
            if (pid == 0){
                execl("/bin/cp", "cp", watchingItem.pathToFile, backupName, NULL);
            }
        }
        //czekaj do następnego sprawdzenia daty
        sleep(watchingItem.interval);
    }

    return howCopies;
}

void clean(struct WatchingItem* watchingItem, int howItem, pid_t* process){
    for(int i = 0; i < howItem; i++){
        free(watchingItem[i].pathToFile);
    }
    free(watchingItem);

    free(process);
}