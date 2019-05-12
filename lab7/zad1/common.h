#define PROJECT_ID 'A'

#define MEM_LINE "MEM_LINE"
#define MEM_LINE_PARAM "MEM_LINE_PARAM"

#define SEM "SEM"

#define TRUCKER 1
#define LOADER 2

#define STANDARD_PERMISSIONS 0666

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

void blockSem(int semId, int type) {
    struct sembuf state;
    state.sem_num = 0;
    state.sem_flg = 0;
    state.sem_op = -1;
    if (semop(semId, &state, 1) == -1) {
        printErrorMessage("Error while block line", type);
        exit(130);
    }
}

void releaseSem(int semId, int type) {
    struct sembuf state;
    state.sem_num = 0;
    state.sem_flg = 0;
    state.sem_op = 1;
    if (semop(semId, &state, 1) == -1) {
        printErrorMessage("Error while release line", type);
        exit(131);
    }
}

int setUpShareMemory(const char * path, size_t size, int index, int type) {
    key_t key;
    int ID;
    char name[100];
    sprintf(name, "./%s", path);

    if ((key = ftok(name, PROJECT_ID)) == (key_t) -1) {
        printErrorMessage("Error while getting unique key", type);
        exit(111);
    }

    if ((ID = shmget(key, size, IPC_CREAT | STANDARD_PERMISSIONS)) == -1) {
        printErrorMessage("Error while creating line", type);
        exit(112);
    }

    return ID;
}

int setUpSemaphore(const char * path, int defaultValue, int type) {
    key_t key;
    int semId;
    char name[100];
    sprintf(name, "./%s", path);

    if ((key = ftok(name, PROJECT_ID)) == -1) {
        printErrorMessage("Error while getting unique semaphore key", type);
        exit(111);
    }
    if ((semId = semget(key, 1, IPC_CREAT | STANDARD_PERMISSIONS)) == -1) {
        printErrorMessage("Error while creating semaphore", type);
        exit(112);
    }

    // initial values
    semSetter.val = defaultValue;
    if (semctl(semId, 0, SETVAL, semSetter) == -1) {
        printErrorMessage("Error while setting value of semaphore", type);
        exit(113);
    }

    return semId;
}

int getSemaphore(const char * path, int type) {
    key_t key;
    int semId;
    char name[100];
    sprintf(name, "./%s", path);

    if ((key = ftok(name, PROJECT_ID)) == -1) {
        printErrorMessage("Error while getting unique semaphore key", type);
        exit(111);
    }
    if ((semId = semget(key, 1, 0)) == -1) {
        printErrorMessage("Error while getting access to semaphore", type);
        exit(112);
    }

    return semId;
}

struct Parcel* getLine(int ptr, int type) {
    struct Parcel* line = (struct Parcel*) shmat(ptr, NULL, 0);
    if(line == (void *)-1) {
        printErrorMessage("Error while attache memory", type);
        exit(140);
    }
    return line;
}

void releaseLine(struct Parcel* line, int type) {
    if (shmdt(line) == -1) {
        printErrorMessage("Error while detach memory", type);
        exit(141);
    }
}

struct LineParams* getLineParams(int ptr, int type) {
    struct LineParams* params = (struct LineParams*) shmat(ptr, NULL, 0);
    if(params == (void *)-1) {
        printErrorMessage("Error while attache memory", type);
        exit(140);
    }

    return params;
}

void releaseLineParams(struct LineParams* params, int type) {
    if (shmdt(params) == -1) {
        printErrorMessage("Error while detach memory", type);
        exit(141);
    }
}

void moveLine(struct Parcel* line, int len) {
    for(int i = 1; i < len; i ++) {
        line[i] = line[i - 1];
    }
    line[0].workerId = 0;
    line[0].weight = 0;
    line[0].timestamp = 0;
}