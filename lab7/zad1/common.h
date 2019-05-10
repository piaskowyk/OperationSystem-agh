#define PROJECT_ID 'M'

#define MEM_LINE '/tmp/MEM_LINE'
#define MEM_TRUCKER_ARG '/tmp/MEM_TRUCKER_ARG'
#define MEM_LINE_PARAM '/tmp/MEM_LINE_PARAM'

#define SEM_LINE '/tmp/SEM_LINE'
#define SEM_TRUCKER_ARG '/tmp/SEM_TRUCKER_ARG'
#define SEM_LINE_PARAM '/tmp/SEM_LINE_PARAM'

#define STANDARD_PERMISSIONS 0660
#define READ_ONLY_FOR_OTHER 0640

struct ShareMemory {
    int mem; // id memory
    int sem; // id semaphor
};

void blockMem(int semId) {
    struct sembuf state;
    state.sem_num = 0;
    state.sem_flg = 0;
    state.sem_op = -1;
    if (semop(semId, state, 1) == -1) {
        throwError("Error while block line", 130);
    }
}

void relaseMem(int semId) {
    struct sembuf state;
    state.sem_num = 0;
    state.sem_flg = 0;
    state.sem_op = 1;
    if (semop (semId, state, 1) == -1) {
        throwError("Error while relase line", 131);
    }
}

int setUpShareMemory(const char * path, size_t size, int permission, int index) {
    key_t key;
    int ID;
    if ((key = ftok(path, PROJECT_ID)) == (key_t) -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while getting unique key (%d).\n", index);
        exit(111);
    }

    if ((ID = shmget(key, size, permission)) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while creating line (%d).\n", index);
        exit(112);
    }

    return ID;
}

int setUpSemaphore(const char * path, int defaultValue, int index) {
    key_t key;
    int semId;
    if ((key = ftok(path, PROJECT_ID)) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while getting unique semaphor key (%d).\n", index);
        exit(111);
    }
    if ((semId = semget(key, 1, IPC_CREAT | STANDARD_PERMISSIONS)) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while creating semaphore (%d).\n", index);
        exit(112);
    }
    // initial values
    semSetter.val = defaultValue;
    if (semctl (semId, 0, SETVAL, sem_attr) == -1) {
        fprintf(stderr, "\033[1;32mTrucker:\033[0m Error while setting value of semaphore (%d).\n", index);
        exit(113);
    }

    return semId;
}