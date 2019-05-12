#define PROJECT_ID 'A'

#define MEM_LINE "MEM_LINE"
#define MEM_LINE_PARAM "MEM_LINE_PARAM"

#define SEM_LINE "SEM_LINE"
#define SEM_LINE_PARAM "SEM_LINE_PARAM"

#define STANDARD_PERMISSIONS 0660

struct ShareMemory {
    int mem; // id memory
    int sem; // id semaphore
};

struct Parcel {
    pid_t workerId;
    long timestamp;
    unsigned int weight;
};

union semun
{
    int val;
    struct semid_ds* buf;
    ushort array [1];
} semSetter;

void blockSem(int semId) {
    struct sembuf state;
    state.sem_num = 0;
    state.sem_flg = 0;
    state.sem_op = -1;
    if (semop(semId, &state, 1) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while block line.\n");
        exit(130);
    }
}

void releaseSem(int semId) {
    struct sembuf state;
    state.sem_num = 0;
    state.sem_flg = 0;
    state.sem_op = 1;
    if (semop(semId, &state, 1) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while release line.\n");
        exit(131);
    }
}

int setUpShareMemory(const char * path, size_t size, int index) {
    key_t key;
    int ID;
    char name[100];
    sprintf(name, "./%s", path);

    if ((key = ftok(name, PROJECT_ID)) == (key_t) -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while getting unique key (%d) - errno: %d\n", index, errno);
        exit(111);
    }

    if ((ID = shmget(key, size, IPC_CREAT | STANDARD_PERMISSIONS)) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while creating line (%d) - errno: %d\n", index, errno);
        exit(112);
    }

    return ID;
}

int setUpSemaphore(const char * path, int defaultValue, int index) {
    key_t key;
    int semId;
    char name[100];
    sprintf(name, "./%s", path);

    if ((key = ftok(name, PROJECT_ID)) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while getting unique semaphore key (%d).\n", index);
        exit(111);
    }
    if ((semId = semget(key, 1, IPC_CREAT | STANDARD_PERMISSIONS)) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while creating semaphore (%d).\n", index);
        exit(112);
    }

    // initial values
    semSetter.val = defaultValue;
    if (semctl(semId, 0, SETVAL, semSetter) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while setting value of semaphore (%d).\n", index);
        exit(113);
    }

    return semId;
}

void setFreeWeightOnLine(unsigned int weight, struct ShareMemory shareMemory) {
    blockSem(shareMemory.sem);
    unsigned int* lineParam = (unsigned int *) shmat(shareMemory.mem, 0, 0);
    lineParam[0] = weight;
    releaseSem(shareMemory.sem);
}

long getTimestamp() {
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    return timestamp.tv_nsec;
}