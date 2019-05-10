#define PROJECT_ID 'M'
#define QUEUE_PERMISSIONS 0660

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