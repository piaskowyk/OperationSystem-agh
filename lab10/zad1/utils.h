#pragma once

#define REGISTER_ACTION 1
#define WORK_DONE_ACTION 2
#define LOGOUT_ACTION 3

#define SUCCESS 1
#define ERROR 0

//server type code
#define DEFAULT_T 0
#define USER_ADDED_T 1
#define INVALID_USER_NAME_T 101
#define USER_ALREADY_EXIST_T 102

struct StringArray {
    unsigned int size;
    char** data;
    int* dataItemLen;
};

struct ClientMessage {
    int type;
    int dataLen;
    int clientNameLen;
    char* data;
    char* clientName;
};

struct ServerMessage {
    int code;
    int type;
    int dataLen;
    char* data;
};

struct Client {
    char * name;
    int free;
};

void printErrorMessage(const char * message, int type);
struct StringArray explode(char* string, long len, char delimer);
void cleanStringArray(struct StringArray * items);
void cleanClientMessage();