#ifndef LAB8_UTILS_H
#define LAB8_UTILS_H

#define BLOCK 1
#define INTERLEAVED 2

#define TIMES_RESULTS "../zad1/Times.txt"

struct StringArray{
    unsigned int size;
    char** data;
    int* dataItemLen;
};

struct IntMatrix {
    int width;
    int height;
    int* data;
};

struct FloatMatrix {
    int width;
    int height;
    float* data;
};

void printErrorMessage(const char * message, int type);
struct StringArray explode(char* string, long len, char delimer);
void cleanStringArray(struct StringArray * items);
void initIntMatrix(struct IntMatrix * matrix);
void initFloatMatrix(struct FloatMatrix * matrix);
void cleanIntMatrix(struct IntMatrix * matrix);
void cleanFloatMatrix(struct FloatMatrix * matrix);
struct timespec getTimestamp();
struct timespec calculateTime(struct timespec startTime, struct timespec stopTime);

#endif //LAB8_UTILS_H