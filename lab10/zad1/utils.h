#pragma once

#define REGISTER_ACTION 1
#define WORK_DONE_ACTION 2
#define LOGOUT_ACTION 3

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

void printErrorMessage(const char * message, int type);
struct StringArray explode(char* string, long len, char delimer);
void cleanStringArray(struct StringArray * items);