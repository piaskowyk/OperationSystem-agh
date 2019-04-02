#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define __USE_XOPEN
#include <time.h>

pid_t pid = -1;
int run = 1;
int tmp = 0;
void handleSIGTSTP(int signalNumber);
void handleSIGINT(int signalNumber);
int check_file_exists(const char * filename);

int main(int argc, char *argv[], char *env[]) {
    if(!check_file_exists("./date.sh")){
        printf("Fatal error: Not found script date.sh");
        exit(100);
    }
    signal(SIGINT, handleSIGINT);

    struct sigaction actionStruct;
    actionStruct.sa_handler = handleSIGTSTP;
    sigemptyset(&actionStruct.sa_mask); 
    sigaddset(&actionStruct.sa_mask, SIGTSTP); 
    actionStruct.sa_flags = 0;
    sigaction(SIGTSTP, &actionStruct, NULL); 
    
    while(1) {
        if(run){
            pid = fork();
            if (pid == 0){
                if(execl("/bin/bash", "bash", realpath("date.sh", NULL), NULL) == -1){
                    printf("Error while starting child process.");
                    exit(101);
                }
            }
        }
        pause();
    }
}

void handleSIGTSTP(int signalNumber) {
    printf(" - Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
    if(run == 0) run = 1;
    else {
        run = 0;
        kill(pid, SIGTSTP);
    }
}

void handleSIGINT(int signalNumber) {
    sigset_t newmask;
    sigset_t oldmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGINT);
    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) perror("Nie udało się zablokować sygnału");

    printf(" - Odebrano sygnał SIGINT\n");
    kill(pid, SIGINT);
    exit(signalNumber);

    if (sigprocmask(SIG_SETMASK, &newmask, NULL) < 0) perror("Nie udało się przywrócić maski sygnałów");
}

int check_file_exists(const char * filename){
    FILE* file = fopen(filename, "r");
    if (file){
        fclose(file);
        return 1;
    }
    return 0;
}