#define PROJECT_ID 'M'
#define QUEUE_PERMISSIONS 0660

#define MAX_CLIENTS_COUNT 20
#define MAX_GROUP_SIZE 10

#define SHIFTID 100

struct message_text {
    int id;
    int additionalArg;
    char buf[256];
};

struct message {
    long message_type;
    struct message_text message_text;
};

#define STOP 10
#define LIST 11
#define FRIENDS 12
#define ADD 13
#define DEL 14
#define INIT 15
#define ECHO 16
#define _2ALL 17
#define _2FRIENDS 18
#define _2ONE 19

#define SERVER_RESPONSE 100

char* typeToStr(int type){
    switch(type){
        case STOP:{
            return "STOP";
        }break;
        case LIST:{
            return "LIST";
        }break;
        case FRIENDS:{
            return "FRIENDS";
        }break;
        case INIT:{
            return "INIT";
        }break;
        case ECHO:{
            return "ECHO";
        }break;
        case _2ALL:{
            return "2ALL";
        }break;
        case _2FRIENDS:{
            return "2FRIENDS";
        }break;
        case _2ONE:{
            return "2ONE";
        }break;
    }

    return "";
}