size_t _array_size;
char* _search_directory;
char* _search_file_name;
char* _tmp_file_name;
char** _operation_results;
size_t _next_index;

int check_file_exists(const char * filename);
int set_tab_size(size_t array_size);
void set_search_directory(char* const search_directory);
void set_search_file_name(char* const search_file_name);
int set_tmp_file_name(char* const tmp_file_name);
int init();
int find();
int load_content_from_tmp_file();
void remove_operation_item(int index);
void clear();