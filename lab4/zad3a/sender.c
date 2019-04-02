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

pid_t catherPid;
unsigned int signalsLimit;
unsigned int mode;

unsigned int receivedSignalsCount = 0;

void handleSignalFromCatcher(int signalNumber, siginfo_t *signal, void *data);
void handleEndSignal(int signalNumber, siginfo_t *signal, void *data);

void sendSignalsKill();
void sendSignalsSigQueue();
void sendSignalsRT();

int main(int argc, char *argv[]) {
    //parging inpput arguments
    int validInput = 1;

    if(argc == 4){
        catherPid = (pid_t)strtol(argv[1], NULL, 0);
        signalsLimit = (unsigned int)strtol(argv[2], NULL, 0);
        mode = (unsigned int)strtol(argv[3], NULL, 0);
    }
    else {
        validInput = 0;
    }

    if(!validInput){
        fputs("\033[1;32mSender:\033[0m Invalid input arguments.\n", stderr);
        exit(101);
    }

    //setting up signals handler and blocking signals
    sigset_t blockInHandler;
    sigset_t blockInHandlerRT;
    sigset_t blockMask;
    sigset_t blockMaskRT;

    sigemptyset(&blockInHandler);
    sigaddset(&blockInHandler, SIGUSR1);
    sigaddset(&blockInHandler, SIGUSR2);

    sigemptyset(&blockInHandlerRT);
    sigaddset(&blockInHandlerRT, SIGRTMIN+1);
    sigaddset(&blockInHandlerRT, SIGRTMIN+2);

    sigfillset(&blockMask);
    sigdelset(&blockMask, SIGUSR1);
    sigdelset(&blockMask, SIGUSR2);

    sigfillset(&blockMaskRT);
    sigdelset(&blockMaskRT, SIGRTMIN+1);
    sigdelset(&blockMaskRT, SIGRTMIN+2);

    struct sigaction actionSignalFromCatcher;
    actionSignalFromCatcher.sa_sigaction = handleSignalFromCatcher;
    actionSignalFromCatcher.sa_flags = SA_SIGINFO;

    struct sigaction actionEndSignal;
    actionEndSignal.sa_sigaction = handleEndSignal;
    actionEndSignal.sa_flags = SA_SIGINFO;

    if(mode == 1 || mode == 2){
        if (sigprocmask(SIG_SETMASK, &blockMask, NULL) < 0) {
            fputs("\033[1;32mSender:\033[0m Unable to set block signal\n", stderr);
            exit(101);
        }

        actionSignalFromCatcher.sa_mask = blockInHandler;
        sigaction(SIGUSR1, &actionSignalFromCatcher, NULL);

        actionEndSignal.sa_mask = blockInHandler;
        sigaction(SIGUSR2, &actionEndSignal, NULL);
    }
    else if(mode == 3){
        if (sigprocmask(SIG_SETMASK, &blockMaskRT, NULL) < 0) {
            fputs("\033[1;32mSender:\033[0m Unable to set block signal\n", stderr);
            exit(101);
        }

        actionSignalFromCatcher.sa_mask = blockInHandlerRT;
        sigaction(SIGRTMIN+1, &actionSignalFromCatcher, NULL);

        actionEndSignal.sa_mask = blockInHandlerRT;
        sigaction(SIGRTMIN+2, &actionEndSignal, NULL);
    }


    //sending signals
    switch (mode){
        case 1: sendSignalsKill();
            break;
        case 2: sendSignalsSigQueue();
            break;
        case 3: sendSignalsRT();
            break;
        default:
            break;
    }

    while(1){
        pause();
    }

    return 0;
}

void handleSignalFromCatcher(int signalNumber, siginfo_t *signal, void *data) {
    receivedSignalsCount++;
    printf("\033[1;32mSender:\033[0m Receive %i signal: %i, from Catcher\n", receivedSignalsCount, signalNumber);
}

void handleEndSignal(int signalNumber, siginfo_t *signal, void *data) {
    printf("\033[1;32mSender:\033[0m Receive END_SIGNAL from Catcher\n");
    printf("\033[1;32mSender: Sender receive %d signals.\033[0m\n", receivedSignalsCount);
    if(mode == 2){
        printf("\033[1;32mSender:\033[0m Sender receive %d of %d signals.\n", receivedSignalsCount, signal->si_int);
    }
    exit(0);
}

void sendSignalsKill() {
    printf("\033[1;32mSender:\033[0m Start sending SIGUSR1 to catcher - using kill().\n");

    for (int i = 0; i < signalsLimit; i++) {
        kill(catherPid, SIGUSR1);
        printf("\033[1;32mSender:\033[0m Sent signal no: %i.\n", i +1);
    }
    kill(catherPid, SIGUSR2);
    
    printf("\033[1;32mSender:\033[0m End sending %i times SIGUSR1 and once SIGUSR2 to catcher.\n", signalsLimit);
}

void sendSignalsSigQueue() {
    printf("\033[1;32mSender:\033[0m Start sending SIGUSR1 to catcher - using sigqueue().\n");

    for (int i = 0; i < signalsLimit; ++i) {
        union sigval signalValue;
        signalValue.sival_int = i + 1;
        if(sigqueue(catherPid, SIGUSR1, signalValue) == 0) {
            printf("\033[1;32mSender:\033[0m Send %d signal to Sender.\n", i+1);
        } else {
            fputs("\033[1;32mSender:\033[0m Unable to send signal to Sender.\n", stderr);
        }
        printf("\033[1;32mSender:\033[0m Sent signal no: %i.\n", i +1);
    }

    kill(catherPid, SIGUSR2);

    printf("\033[1;32mSender:\033[0m End sending %i times SIGUSR1 and once SIGUSR2 to catcher.\n", signalsLimit);
}

void sendSignalsRT() {
    printf("\033[1;32mSender:\033[0m Start sending SIGRTMIN+1 to catcher.\n");

    for (int i = 0; i < signalsLimit; ++i) {
        kill(catherPid, SIGRTMIN+1);
        printf("\033[1;32mSender:\033[0m Sent signal no: %i.\n", i +1);
    }

    kill(catherPid, SIGRTMIN+2);

    printf("\033[1;32mSender:\033[0m End sending %i times SIGRTMIN+1 and once SIGUSR2 to catcher.\n", signalsLimit);
}