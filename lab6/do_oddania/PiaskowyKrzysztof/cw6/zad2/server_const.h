#define SERVER_QUEUE_NAME "/queue_server"

#define PROJECT_ID 'M'
#define QUEUE_PERMISSIONS 0660

#define MAX_CLIENTS_COUNT 20
#define MAX_GROUP_SIZE 10
#define MAX_MESSAGE_SIZE 512
#define MAX_MESSAGES 10

#define SHIFTID 100

#define STOP 10
#define LIST 11
#define FRIENDS 12

#define INIT 15
#define ECHO 16
#define _2ALL 17
#define _2FRIENDS 18
#define _2ONE 19

#define ADD 23
#define DEL 24

#define SHUTDOWN 30

#define SERVER_RESPONSE 100
#define ERROR 500
#define SERVER_ID -10

struct message_text {
    int id;
    int additionalArg;
    char buf[256];
};

struct message {
    long message_type;
    struct message_text message_text;
};

struct StringArray{
    unsigned int size;
    char** data;
};

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
        case ADD:{
            return "ADD";
        }break;
        case DEL:{
            return "DEL";
        }break;
    }

    return "";
}

struct StringArray explode(char* string, long len, char delimer) {
    struct StringArray itemsArray;
    char** items = NULL;
    int itemsCount = 0;

    itemsArray.size = 0;
    itemsArray.data = NULL;

    if(len == 0 || string == NULL) return itemsArray;

    itemsCount++;
    for(long i = 0; i < len; i++){
        if(string[i] == delimer) {
            itemsCount++;
        }
    }

    items = calloc(itemsCount, sizeof(char*));

    int indexGlob, indexStart;
    indexGlob = indexStart = 0;
    for(int i = 0; i < itemsCount; i++) {
        indexStart = indexGlob;
        while(indexGlob < len && string[indexGlob] != delimer) indexGlob++;

        if(indexGlob == indexStart){
            itemsCount--;
            i--;
            continue;
        }
        items[i] = calloc(indexGlob - indexStart + 1, sizeof(char));
        memcpy(items[i], string + indexStart, (indexGlob - indexStart) * sizeof(char));
        indexGlob++;
    }
    
    itemsArray.size = itemsCount;
    itemsArray.data = items;

    return itemsArray;
}