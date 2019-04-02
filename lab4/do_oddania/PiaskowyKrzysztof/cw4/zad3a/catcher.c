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

void handleReceiver(int signalNumber, siginfo_t *si, void *data);
void handleSender(int signalNumber, siginfo_t *si, void *data);

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
    sigset_t blockMaskRT;
    sigset_t blockInHandler;
    sigset_t blockInHandlerRT;

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

    struct sigaction actionReceiver;
    actionReceiver.sa_sigaction = handleReceiver;
    actionReceiver.sa_flags = SA_SIGINFO;

    struct sigaction actionSender;
    actionSender.sa_sigaction = handleSender;
    actionSender.sa_flags = SA_SIGINFO;

    if(mode == 1 || mode == 2){
        if (sigprocmask(SIG_SETMASK, &blockMask, NULL) < 0) {
            fputs("\033[1;33mCatcher:\033[0m Unable to set block signal\n", stderr);
            exit(101);
        }

        //signal initialisation
        actionReceiver.sa_mask = blockInHandler;
        sigaction(SIGUSR1, &actionReceiver, NULL);

        actionSender.sa_mask = blockInHandler;
        sigaction(SIGUSR2, &actionSender, NULL);
    }
    else if(mode == 3){
        if (sigprocmask(SIG_SETMASK, &blockMaskRT, NULL) < 0) {
            fputs("\033[1;33mCatcher:\033[0m Unable to set block signal\n", stderr);
            exit(101);
        }

        //signal initialisation
        actionReceiver.sa_mask = blockInHandlerRT;
        sigaction(SIGRTMIN+1, &actionReceiver, NULL);

        actionSender.sa_mask = blockInHandlerRT;
        sigaction(SIGRTMIN+2, &actionSender, NULL);
    }

    printf("\033[1;33mCatcher:\033[0m Catcher PID: %d\n", getpid());

    while(1) {
        pause();
    }

    return 0;
}

void handleReceiver(int signalNumber, siginfo_t *signal, void *data) {
    sigCount++;
    printf("\033[1;33mCatcher:\033[0m Receive signal: %i, no %i, from process with pid: %i\n", signalNumber, sigCount, signal->si_pid);
}

void handleSender(int signalNumber, siginfo_t *signal, void *data) {
    printf("\033[1;33mCatcher:\033[0m Receive END_SIGNAL from process with pid: %d\n", signal->si_pid);
    printf("\033[1;33mCatcher: Catcher receive %d signals.\033[0m\n", sigCount);
    printf("\033[1;33mCatcher:\033[0m Catcher start sending signals to Sender.\n");

    for (int i = 0; i < sigCount; ++i) {
        if(mode == 1) {
            kill(signal->si_pid, SIGUSR1);
        }
        else if(mode == 2) {
            union sigval signalValue;
            signalValue.sival_int = i + 1;

            if(sigqueue(signal->si_pid, SIGUSR1, signalValue) == 0) {
                printf("\033[1;33mCatcher:\033[0m Send signal no %d to Sender.\n", i+1);
            } else {
                fputs("\033[1;33mCatcher:\033[0m Unable to send signal to Sender.\n", stderr);
            }
        }
        else if(mode == 3){
            kill(signal->si_pid, SIGRTMIN+1);
        }
    }

    if(mode == 1){
        kill(signal->si_pid, SIGUSR2);
    }
    else if(mode == 2) {
        union sigval signalValue;
        signalValue.sival_int = sigCount;
        sigqueue(signal->si_pid, SIGUSR2, signalValue);
    }
    else if(mode == 3) {
        kill(signal->si_pid, SIGRTMIN+2);
    }

    printf("\033[1;33mCatcher:\033[0m End sending signal to Sender.\n");
}
