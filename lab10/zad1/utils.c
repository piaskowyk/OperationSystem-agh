#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "utils.h"

void printErrorMessage(const char * message, int type) {
    fprintf(stderr, "\033[1;34mError:\033[0m %s. errno: %d\n", message, errno);
    if(errno > 0) {
        perror("Errno info");
    }
    if(type > 0){
        exit(type);
    }
}