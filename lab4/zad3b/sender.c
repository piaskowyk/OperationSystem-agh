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
unsigned int receivedConfirmationCount = 0;
unsigned int sendSignalCount = 0;

int senderMode = 1;
int lastSignal = 0;

void handleForFirstSignal(int signalNumber, siginfo_t *si, void *data);
void handleForSecondSignal(int signalNumber, siginfo_t *si, void *data);

void sendSignalsKill(int lastSig);
void sendSignalsSigQueue(int lastSig);
void sendSignalsRT(int lastSig);

void sendSignal(int lastSig);

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

    //blocking signals
    sigset_t blockMask;
    sigset_t blockInHandler;

    sigemptyset(&blockInHandler);
    sigaddset(&blockInHandler, SIGUSR1);
    sigaddset(&blockInHandler, SIGUSR2);
    sigaddset(&blockInHandler, SIGRTMIN+1);
    sigaddset(&blockInHandler, SIGRTMIN+2);

    sigfillset(&blockMask);
    sigdelset(&blockMask, SIGUSR1);
    sigdelset(&blockMask, SIGUSR2);
    sigdelset(&blockMask, SIGRTMIN+1);
    sigdelset(&blockMask, SIGRTMIN+2);

    struct sigaction actionReceiver;
    actionReceiver.sa_sigaction = handleForFirstSignal;
    actionReceiver.sa_flags = SA_SIGINFO;
    actionReceiver.sa_mask = blockInHandler;

    struct sigaction actionSender;
    actionSender.sa_sigaction = handleForSecondSignal;
    actionSender.sa_flags = SA_SIGINFO;
    actionSender.sa_mask = blockInHandler;

    if (sigprocmask(SIG_SETMASK, &blockMask, NULL) < 0) {
        fputs("\033[1;32mSender:\033[0m Unable to set block signal\n", stderr);
        exit(101);
    }

    if(mode == 1 || mode == 2){
        sigaction(SIGUSR1, &actionReceiver, NULL);
        sigaction(SIGUSR2, &actionSender, NULL);
    }
    else if(mode == 3){
        sigaction(SIGUSR1, &actionReceiver, NULL);
        sigaction(SIGRTMIN+1, &actionReceiver, NULL);
        sigaction(SIGRTMIN+2, &actionSender, NULL);
    }

    //sending signals
    sendSignal(0);

    while(1){
        pause();
    }

    return 0;
}

void handleForFirstSignal(int signalNumber, siginfo_t *signal, void *data) {
    if(!senderMode){
        receivedSignalsCount++;
        printf("\033[1;32mSender:\033[0m Receive %i signal: %i, from Catcher\n", receivedSignalsCount, signalNumber);

        //send signal with confirmation
        kill(signal->si_pid, SIGUSR1);
        printf("\033[1;32mSender:\033[0m Send confirmation signal no %i\n", receivedSignalsCount);
    }
    else {
        receivedConfirmationCount++;
        printf("\033[1;32mSender:\033[0m Receive confirmation signal no %i\n", receivedConfirmationCount);
        if(receivedConfirmationCount != signalsLimit){
            //send next signal
            sendSignal(0);
        }
        else {
            //send ending
            printf("\033[1;32mSender:\033[0m Send ending signal to Catcher.\n");
            sendSignal(1);
            senderMode = 0;
        }
    }
}

void handleForSecondSignal(int signalNumber, siginfo_t *signal, void *data) {
    printf("\033[1;32mSender:\033[0m Receive END_SIGNAL from Catcher\n");
    printf("\033[1;32mSender: Sender receive %d signals.\033[0m\n", receivedSignalsCount);
    if(mode == 2){
        printf("\033[1;32mSender:\033[0m Sender receive %d of %d signals.\n", receivedSignalsCount, signal->si_int);
    }

    //send signal with confirmation
    kill(signal->si_pid, SIGUSR1);
    exit(0);
}

void sendSignalsKill(int lastSig) {
    if(sendSignalCount == 0) {
        printf("\033[1;32mSender:\033[0m Start sending SIGUSR1 to catcher - using kill().\n");
    }

    if(signalsLimit > sendSignalCount){
        sendSignalCount++;
        kill(catherPid, SIGUSR1);
        printf("\033[1;32mSender:\033[0m Send signal no: %i.\n", sendSignalCount);
    }

    if(lastSig) {
        kill(catherPid, SIGUSR2);
        printf("\033[1;32mSender: End sending. Send %i signals.\033[0m\n", sendSignalCount);
    }
}

void sendSignalsSigQueue(int lastSig) {
    if(sendSignalCount == 0) {
        printf("\033[1;32mSender:\033[0m Start sending SIGUSR1 to catcher - using sigqueue().\n");
    }

    if(signalsLimit > sendSignalCount){
        sendSignalCount++;
        union sigval signalValue;
        signalValue.sival_int = sendSignalCount;
        if(sigqueue(catherPid, SIGUSR1, signalValue) == 0) {
            printf("\033[1;32mSender:\033[0m Send %d signal to Catcher.\n", sendSignalCount);
        } else {
            fputs("\033[1;32mSender:\033[0m Unable to send signal to Sender.\n", stderr);
        }
    }

    if(lastSig) {
        kill(catherPid, SIGUSR2);
        printf("\033[1;32mSender: End sending. Send %i signals.\033[0m\n", sendSignalCount);
    }
}

void sendSignalsRT(int lastSig) {
    if(sendSignalCount == 0) {
        printf("\033[1;32mSender:\033[0m Start sending SIGRTMIN+1 to catcher.\n");
    }

    if(signalsLimit > sendSignalCount) {
        sendSignalCount++;
        kill(catherPid, SIGRTMIN+1);
        printf("\033[1;32mSender:\033[0m Sent signal no: %i.\n", sendSignalCount);
    }

    if(lastSig) {
        kill(catherPid, SIGRTMIN+2);
        printf("\033[1;32mSender: End sending. Send %i signals.\033[0m\n", sendSignalCount);
    }
}

void sendSignal(int lastSig) {
    switch (mode){
        case 1: sendSignalsKill(lastSig);
            break;
        case 2: sendSignalsSigQueue(lastSig);
            break;
        case 3: sendSignalsRT(lastSig);
            break;
        default:
            break;
    }
}