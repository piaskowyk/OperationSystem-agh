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
    pid_t pid;
};

int endProgram = 0;
pid_t parentPid;
pid_t* process;
struct WatchingItem* watchingItems;
int watchingFileCount = 0;

int reciveSIGUSR1 = 0;
int reciveSIGUSR2 = 0;

int watcherCopyViaProg(struct WatchingItem watchingItem);
void clean();

void list();
void stopPid(pid_t pid);
void stopAll();
void startPid(pid_t pid);
void startAll();
void end();

void handleSIGINT(int signalNumber);
void handleSIGUSR1(int signalNumber);
void handleSIGUSR2(int signalNumber);

int main(int argc, char *argv[]) {
    parentPid = getpid();

    //signal initialisation
    struct sigaction action;
    action.sa_handler = handleSIGINT;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGINT); 
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    struct sigaction actionSU1;
    actionSU1.sa_handler = handleSIGUSR1;
    sigemptyset(&actionSU1.sa_mask);
    sigaddset(&actionSU1.sa_mask, SIGUSR1); 
    actionSU1.sa_flags = 0;
    sigaction(SIGUSR1, &actionSU1, NULL);

    struct sigaction actionSU2;
    actionSU2.sa_handler = handleSIGUSR2;
    sigemptyset(&actionSU2.sa_mask);
    sigaddset(&actionSU2.sa_mask, SIGUSR2); 
    actionSU2.sa_flags = 0;
    sigaction(SIGUSR2, &actionSU2, NULL);

    //parging inpput arguments
    int validInput = 1;
    char* listFileName = "";

    if(argc == 2){
        listFileName = argv[1];
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
    watchingFileCount = fileLinesCount;

    //parsowanie pliku lista
    watchingItems = calloc((size_t )fileLinesCount, sizeof(struct WatchingItem));
    int index = 0;
    for(int wathingItemIndex = 0; wathingItemIndex < watchingFileCount; wathingItemIndex++) {
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

        watchingItems[wathingItemIndex].interval = (unsigned int)strtol(intervalStr, NULL, 0);
        index = indexStart + len + 1;
        free(intervalStr);
    }
    free(fileListContent);

    //tworzenie katalogu archiwum jeśli nie isitnieje
    struct stat st = {0};
    if (stat("./archiwum", &st) == -1) {
        mkdir("./archiwum", 0700);
    }

    //tworzenie procesów potomnych do monitorowania plików
    process = calloc((size_t )watchingFileCount, sizeof(pid_t));
    for(int i = 0; i < watchingFileCount; i++){
        pid_t pid;
        pid = fork();
        if (pid > 0) {
            process[i] = pid;
            watchingItems[i].pid = pid;
        }
        else if (pid == 0){
            int result = watcherCopyViaProg(watchingItems[i]);
            clean(watchingItems, watchingFileCount, process);
            return result;
        }
        else {
            fputs("Error while starting process\n", stderr);
        }
    }

    //nasłuchiwanie na komentu użytkownika
    int commandLen = 20;
    while(1) {
        char* command = calloc(commandLen, sizeof(char));  
        printf(">> ");
        int oneChar = 0; 
        char *ptr = command;
        while ((oneChar = fgetc(stdin)) != '\n') {
            if(command + commandLen > ptr) {
                (*ptr++) = (char)oneChar; 
            }
            else {
                command[19] = '\0';
            }
        } 
        
        //parsowanie komendy wejściowej
        char* commandArg1 = calloc(commandLen, sizeof(char));
        int commandArg1Index = 0;  
        char* commandArg2 = calloc(commandLen, sizeof(char));
        int commandArg2Index = 0; 
        int startInput = 0; 
        int nextIndex = 0; 
        for(int i = 0; i < 20; i++){
            nextIndex++;
            if(!startInput && command[i] == ' ') continue;
            startInput = 1;
            if(command[i] == ' ') break;

            commandArg1[commandArg1Index++] = command[i];            
        }

        startInput = 0;
        for(int i = nextIndex; i < 20; i++){
            if(!startInput && command[i] == ' ') continue;
            startInput = 1;
            if(command[i] == ' ') break;

            commandArg2[commandArg2Index++] = command[i];            
        }
        
        //rozpoznawanie komendy
        if(strcmp(commandArg1, "list") == 0) {
            list();
        }
        else if(strcmp(commandArg1, "stop") == 0) {
            if(strcmp(commandArg2, "all") == 0){
                stopAll();
            }
            else {
                pid_t targetPid = (pid_t)strtol(commandArg2, NULL, 0);
                stopPid(targetPid);
            }
        }
        else if(strcmp(commandArg1, "start") == 0) {
            if(strcmp(commandArg2, "all") == 0){
                startAll();
            }
            else {
                pid_t targetPid = (pid_t)strtol(commandArg2, NULL, 0);
                startPid(targetPid);
            }
        }
        else if(strcmp(commandArg1, "end") == 0) {
            end();
            endProgram = 1;
        }
        else {
            printf("Command \"%s\" not exist.\n", command);
        }

        free(commandArg1);
        free(commandArg2);
        free(command);

        if(endProgram) break;
    }

    return 0;
}

int watcherCopyViaProg(struct WatchingItem watchingItem){
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
    while(1){
        if(endProgram) break;

        if(reciveSIGUSR1 && !endProgram){
            while(!reciveSIGUSR2 && !endProgram){
                pause();
            }
        }

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
            size_t backupNameLen = fileNameLen + 21 + 11;
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

void clean(){
    for(int i = 0; i < watchingFileCount; i++){
        free(watchingItems[i].pathToFile);
    }
    free(watchingItems);
    free(process);
}

void list() {
    for(int i = 0; i < watchingFileCount; i++){
        printf("Process (PID: %d) watching on file: %s\n",
                watchingItems[i].pid,
                watchingItems[i].pathToFile);
    }
}

void stopPid(pid_t pid) {
    printf("Stopping process %d\n", pid);
    kill(pid, SIGUSR1);
    printf("Done\n");
}

void stopAll() {
    printf("Stoping all process.\n");
    for(int i = 0; i < watchingFileCount; i++) {
        kill(watchingItems[i].pid, SIGUSR1);
    }
    printf("Done\n");
}

void startPid(pid_t pid) {
    printf("Starting process %d\n", pid);
    kill(pid, SIGUSR2);
    printf("Done\n");
}
 
void startAll() {
    printf("Starting all process.\n");
    for(int i = 0; i < watchingFileCount; i++) {
        kill(watchingItems[i].pid, SIGUSR2);
    }
    printf("Done\n");
}

void end() {
    printf("Ending all of process.\n");

    for(int i = 0; i < watchingFileCount; i++) {
        kill(watchingItems[i].pid, SIGINT);
    }

    for(int i = 0; i < watchingFileCount; i++){
        pid_t wait;
        int status = 0;
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
    clean(watchingItems, watchingFileCount, process);
    printf("Done\n");
}

void handleSIGINT(int signalNumber) {
    if(parentPid == getpid()){
        printf(" - Recive signal SIGINT\n");
        end();
        exit(signalNumber);
    }
    endProgram = 1;
}

void handleSIGUSR1(int signalNumber) {
    reciveSIGUSR1 = 1;
    reciveSIGUSR2 = 0;
}

void handleSIGUSR2(int signalNumber) {
    reciveSIGUSR1 = 0;
    reciveSIGUSR2 = 1;
}