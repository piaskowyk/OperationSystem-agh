#define PROJECT_ID 'A'

#define MEM_LINE "memline"
#define MEM_LINE_PARAM "memlineparam"

#define SEM "sem"

#define TRUCKER 1
#define LOADER 2

#define STANDARD_PERMISSIONS 0666

#define DEBUG 1

struct ShareMemory {
    int mem; // id memory
    int sem; // id semaphore
};

struct Parcel {
    pid_t workerId;
    long timestamp;
    unsigned int weight;
};

struct LineParams {
    unsigned int freeWeight;
    unsigned int freePlaces;
    unsigned int len;

    unsigned int mode;
    unsigned int loadersCount;
    unsigned int loadersCountEndWord;
};

union semun
{
    int val;
    struct semid_ds* buf;
    ushort array [1];
} semSetter;

long getTimestamp() {
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    return timestamp.tv_nsec;
}

void printErrorMessage(const char * message, int type) {
    if(type == TRUCKER) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m %ld, %s. errno: %d\n", getTimestamp(), message, errno);
    }
    else {;
        fprintf(stderr, "\033[1;34mLoader:\033[0m %ld, %s. errno: %d\n", getTimestamp(), message, errno);
    }
}

void blockSem(sem_t * semId, int type) {
    if(sem_wait(semId) < 0) {
        printErrorMessage("Error while block line", type);
        exit(130);
    }
}

void releaseSem(sem_t * semId, int type) {
    if(sem_post(semId) < 0) {
        printErrorMessage("Error while release line", type);
        exit(130);
    }
}

int setUpShareMemory(const char * path, unsigned long size, int index, int type) {
    int ID;

    ID = shm_open(path, O_RDWR | O_CREAT | O_EXCL, S_IRWXU | S_IRWXG);
    if (ID == -1) {
        printErrorMessage("Error while creating line", type);
        exit(112);
    }

    if (ftruncate(ID, size) == -1) {
        printErrorMessage("Error while set up size of line", type);
        exit(112);
    }

    return ID;
}

sem_t* setUpSemaphore(const char * path, int type) {
    sem_t* semId;

    semId = sem_open(path, O_RDWR | O_CREAT | O_EXCL, S_IRWXU | S_IRWXG, 1);
    if(semId == SEM_FAILED) {
        printErrorMessage("Error while creating semaphore", type);
        exit(112);
    }

    return semId;
}

struct Parcel* getLine(int ptr, int size, int type) {
    struct Parcel* line = (struct Parcel*) mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, ptr, 0);
    if (line == (void *) -1) {
        printErrorMessage("Error while attache memory", type);
        exit(140);
    }

    return line;
}

void releaseLine(struct Parcel* line, int size, int type) {
    if (munmap(line, size) == -1) {
        printErrorMessage("Error while detach memory", type);
        exit(141);
    }
}

struct LineParams* getLineParams(int ptr, int size, int type) {
    struct LineParams* params = (struct LineParams*) mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, ptr, 0);
    if(params == (void *)-1) {
        printErrorMessage("Error while attache memory", type);
        exit(140);
    }

    return params;
}

void releaseLineParams(struct LineParams* params, int size, int type) {
    if (munmap(params, size) == -1) {
        printErrorMessage("Error while detach memory", type);
        exit(141);
    }
}

void moveLine(struct Parcel* line, int len) {
    for(int i = len - 1; i > 0; i--) {
        line[i] = line[i - 1];
    }
    line[0].workerId = 0;
    line[0].weight = 0;
    line[0].timestamp = 0;
}