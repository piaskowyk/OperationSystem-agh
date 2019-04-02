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

int sigCount = 0;
int mode = 0;
int sendSigCount = 0;
int confirmationSigCount = 0;
int receiverMode = 1;
int lastSign = 0;
pid_t senderPid;

void handleForFirstSignal(int signalNumber, siginfo_t *si, void *data);
void handleForSecondSignal(int signalNumber, siginfo_t *si, void *data);

void sendSignal(int lastSignal);

int main(int argc, char *argv[]) {
    int valid_input = 1;
    //parsing input arguments
    mode = (unsigned int)strtol(argv[1], NULL, 0);

    if(mode != 1 && mode != 2 && mode != 3){
        valid_input = 0;
    }

    if(!valid_input){
        fputs("\033[1;33mCatcher:\033[0m Invalid input arguments.\n", stderr);
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
        fputs("\033[1;33mCatcher:\033[0m Unable to set block signal\n", stderr);
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

    printf("\033[1;33mCatcher:\033[0m Catcher PID: %d\n", getpid());

    while(1) {
        pause();
    }

    return 0;
}

void handleForFirstSignal(int signalNumber, siginfo_t *si, void *data) {
    if(receiverMode){
        sigCount++;
        printf("\033[1;33mCatcher:\033[0m Receive signal: %i, no %i, from process with pid: %i\n", signalNumber, sigCount, si->si_pid);

        //send signal with confirmation
        kill(si->si_pid, SIGUSR1);
        printf("\033[1;33mCatcher:\033[0m Send confirmation signal no %i\n", sigCount);
    }
    else {
        //sending mode
        senderPid = si->si_pid;
        if (confirmationSigCount != sigCount) {
            confirmationSigCount++;
            printf("\033[1;33mCatcher:\033[0m Receive confirmation signal no %i\n", confirmationSigCount);
            sendSignal(0);
        }
        else {
            printf("\033[1;33mCatcher: End sending. Send %i signals.\033[0m\n", sendSigCount);
            exit(0);
        }
    }
}

void handleForSecondSignal(int signalNumber, siginfo_t *si, void *data) {
    printf("\033[1;33mCatcher:\033[0m Receive END_SIGNAL from process with pid: %d\n", si->si_pid);
    printf("\033[1;33mCatcher: Catcher receive %d signals.\033[0m\n", sigCount);
    printf("\033[1;33mCatcher:\033[0m Catcher start sending signals to Sender.\n");

    //chance mode program to sender
    receiverMode = 0;
    senderPid = si->si_pid;
    sendSignal(0);
}

void sendSignal(int lastSignal) {
    if(sendSigCount < sigCount){
        sendSigCount++;

        if(mode == 1) {
            kill(senderPid, SIGUSR1);
        }
        else if(mode == 2) {
            union sigval signalValue;
            signalValue.sival_int = sendSigCount;
            sigqueue(senderPid, SIGUSR1, signalValue);
        }
        else if(mode == 3){
            kill(senderPid, SIGRTMIN+1);
        }
        printf("\033[1;33mCatcher:\033[0m Sent signal no: %i.\n", sendSigCount);
    }
    else {
        if(mode == 1){
            kill(senderPid, SIGUSR2);
        }
        else if(mode == 2) {
            union sigval signalValue;
            signalValue.sival_int = 0;
            sigqueue(senderPid, SIGUSR2, signalValue);
        }
        else if(mode == 3) {
            kill(senderPid, SIGRTMIN+2);
        }

        printf("\033[1;33mCatcher:\033[0m Send ending signal to Sender.\n");
    }
}