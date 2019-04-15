#define PROJECT_ID 'M'
#define QUEUE_PERMISSIONS 0660

#define MAX_CLIENTS_COUNT 100

struct message_text {
    int qid;
    char buf[200];
};

struct message {
    long message_type;
    struct message_text message_text;
};

#define STOP 10
#define LIST 11
#define FRIENDS 12
#define INIT 13
#define ECHO 14
#define _2ALL 15
#define _2FRIENDS 16
#define _2ONE 17