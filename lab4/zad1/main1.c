#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#define __USE_XOPEN
#include <time.h>

int run = 1;
void handleSIGTSTP(int signalNumber);
void handleSIGINT(int signalNumber);

int main(int argc, char *argv[], char *env[]) {
    signal(SIGINT, handleSIGINT);

    struct sigaction actionStruct;
    actionStruct.sa_handler = handleSIGTSTP;
    sigemptyset(&actionStruct.sa_mask); 
    sigaddset(&actionStruct.sa_mask, SIGTSTP); 
    actionStruct.sa_flags = 0;
    sigaction(SIGTSTP, &actionStruct, NULL); 

    __time_t now;

    while(1) {
        if(run){
            time(&now);
            char* date = calloc(20, sizeof(char));
            strftime(date, 21, "%d-%m-%Y_%H:%M:%S", localtime(&now));
            printf("Data: %s\n", date);
            free(date);
            sleep(1);
        }
        else {
            pause();
        }
    }
    
}

void handleSIGTSTP(int signalNumber) {
    printf(" - Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
    if(run == 0) run = 1;
    else run = 0;
}

void handleSIGINT(int signalNumber) {
    printf(" - Odebrano sygnał SIGINT\n");
    exit(signalNumber);
}