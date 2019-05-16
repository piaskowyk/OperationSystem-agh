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

void *print_message_function( void *ptr );

int main(int argc, char *argv[], char *env[]) {

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