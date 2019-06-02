#pragma once

struct StringArray{
    unsigned int size;
    char** data;
    int* dataItemLen;
};

void printErrorMessage(const char * message, int type);
struct StringArray explode(char* string, long len, char delimer);
void cleanStringArray(struct StringArray * items);