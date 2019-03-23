#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/times.h>
#include <time.h>
#include <zconf.h>
#include "../zad1/find_operation.h"

enum CommandType{
    Remove_block,
    Search_directory
};

struct SearchCommand{
    char* dir_name;
    char* file_name;
    char* tmp_file_name;
};

struct Command{
    enum CommandType command_type;
    float time_in_kernel_mode;
    float time_in_user_mode;
    long real_time_s;
    long real_time_ns;
};

int main(int argc, char *argv[]) {
    long clock_tick_speed = sysconf(_SC_CLK_TCK);
    struct timespec start_program, stop_program;
    struct tms start_cpu, stop_cpu;

    clock_gettime(CLOCK_REALTIME, &start_program);
    times(&start_cpu);

    int valid_input = 1;
    long array_size = -1;
    char* end_ptr = NULL;

    size_t commands_index = 0;
    size_t commands_size = 10;
    struct Command* commands = calloc(commands_size, sizeof(struct Command));

    size_t remove_cmd_index = 0;
    size_t remove_cmd_size = 10;
    size_t* remove_cmd = calloc(remove_cmd_size, sizeof(size_t));

    size_t search_cmd_index = 0;
    size_t search_cmd_size = 10;
    struct SearchCommand* search_cmd = calloc(search_cmd_size, sizeof(struct SearchCommand));

    for (int i = 0; i < argc; i++) {
        if(strncmp(argv[i], "create_table", 12) == 0){
            if(array_size == -1 && i < argc - 1){
                array_size = strtol(argv[i+1], &end_ptr, 0);
                i++;
            }
            else{
                valid_input = 0;
                break;
            }
        }

        if(strncmp(argv[i], "remove_block", 12) == 0){
            if(i < argc - 1){
                long index = strtol(argv[i+1], &end_ptr, 0);

                if(commands_index >= commands_size){
                    commands_size += 10;
                    commands = realloc(commands, (commands_size) * sizeof(struct Command));
                }
                commands[commands_index].command_type = Remove_block;
                commands_index++;

                if(remove_cmd_index >= remove_cmd_size){
                    remove_cmd_size += 10;
                    remove_cmd = realloc(remove_cmd, (remove_cmd_size) * sizeof(size_t));
                }
                remove_cmd[remove_cmd_index] = (size_t)index;
                remove_cmd_index++;
                i++;
            }
            else{
                valid_input = 0;
                break;
            }
        }

        if(strncmp(argv[i], "search_directory", 16) == 0){
            if(i < argc - 3){
                if(commands_index >= commands_size){
                    commands_size += 10;
                    commands = realloc(commands, (commands_size) * sizeof(struct Command));
                }
                commands[commands_index].command_type = Search_directory;
                commands_index++;

                if(search_cmd_index >= search_cmd_size){
                    search_cmd_size += 10;
                    search_cmd = realloc(search_cmd, (search_cmd_size) * sizeof(struct SearchCommand));
                }
                search_cmd[search_cmd_index].dir_name = argv[i+1];
                search_cmd[search_cmd_index].file_name = argv[i+2];
                search_cmd[search_cmd_index].tmp_file_name = argv[i+3];
                search_cmd_index++;
                i += 3;
            }
            else{
                valid_input = 0;
                break;
            }
        }
    }

    if(array_size <= 0){
        valid_input = 0;
        printf("You must entry table size.\n");
    }

    if(array_size > 65535){//max value of size_t
        valid_input = 0;
        printf("Too large array size.\n");
    }

    //check validation input arguments
    if(valid_input == 0){
        free(commands);
        free(remove_cmd);
        free(search_cmd);
        clear();

        fputs("Wrong input arguments.\n", stderr);
        exit(1);
    }

    //execute input command
    if( !set_tab_size((size_t)array_size) || !init() ) exit(100);

    int cmd_rm_index = 0;
    int cmd_search_index = 0;
    for(int i = 0; i < commands_index; i++){
        if(commands[i].command_type == Search_directory){
            //init timers
            struct timespec start_operation, stop_operation;
            struct tms start_operation_cpu, stop_operation_cpu;

            clock_gettime(CLOCK_REALTIME, &start_operation);
            times(&start_operation_cpu);

            //start operation
            set_search_directory(search_cmd[cmd_search_index].dir_name);
            set_search_file_name(search_cmd[cmd_search_index].file_name);
            if(!set_tmp_file_name(search_cmd[cmd_search_index].tmp_file_name)) exit(101);

            if(find() < 0) exit(102);
            if(load_content_from_tmp_file() == -1) exit(103);

            //save timer status
            times(&stop_operation_cpu);
            clock_gettime(CLOCK_REALTIME, &stop_operation);

            commands[i].real_time_s = stop_operation.tv_sec - start_operation.tv_sec;
            commands[i].real_time_ns = stop_operation.tv_nsec - start_operation.tv_nsec;
            commands[i].time_in_user_mode = (float)(stop_operation_cpu.tms_cstime - start_operation_cpu.tms_cstime)/clock_tick_speed;
            commands[i].time_in_kernel_mode = (float)(stop_operation_cpu.tms_cstime - start_operation_cpu.tms_cstime)/clock_tick_speed;

            cmd_search_index++;
        }

        if(commands[i].command_type == Remove_block){
            remove_operation_item(cmd_rm_index);
            cmd_rm_index++;
        }
    }

    times(&stop_cpu);
    clock_gettime(CLOCK_REALTIME, &stop_program);

    //printing report
    printf("Report for arguments: ");
    for(int i = 1; i < argc; i++){
        printf("%s ", argv[i]);
    }
    printf("\n");
    printf("Array size: %lu\n", array_size);
    printf("Number of search operations: %lu\n", search_cmd_index);
    printf("Number of remove operations: %lu\n", remove_cmd_index);
    printf("Real time executed program: %lu s\n", (stop_program.tv_sec - start_program.tv_sec));
    printf("Real time executed program: %lu ns\n", (stop_program.tv_nsec - start_program.tv_nsec));
    printf("Time in user mode executed program: %f s\n", (float)(stop_cpu.tms_cutime - start_cpu.tms_cutime)/clock_tick_speed);
    printf("Time in kernel mode executed program: %f s\n", (float)(stop_cpu.tms_cstime - start_cpu.tms_cstime)/clock_tick_speed);

    long total_search_time_RT_s = 0;
    long total_search_time_RT_ns = 0;
    float total_search_time_UM = 0;
    float total_search_time_KM = 0;

    long total_remove_time_RT_s = 0;
    long total_remove_time_RT_ns = 0;
    float total_remove_time_UM = 0;
    float total_remove_time_KM = 0;

    cmd_rm_index = 0;
    cmd_search_index = 0;
    for(int i = 0; i < commands_index; i++){
        printf("\n");
        if(commands[i].command_type == Search_directory){
            total_search_time_RT_s += commands[i].real_time_s;
            total_search_time_RT_ns += commands[i].real_time_ns;
            total_search_time_UM += commands[i].time_in_user_mode;
            total_search_time_KM += commands[i].time_in_kernel_mode;

            printf("Operation: search_directory, Arg: %s, %s, %s\n",
                    search_cmd[cmd_search_index].dir_name,
                    search_cmd[cmd_search_index].file_name,
                    search_cmd[cmd_search_index].tmp_file_name
                   );

            cmd_search_index++;
        }

        if(commands[i].command_type == Remove_block){
            total_search_time_RT_s += commands[i].real_time_s;
            total_search_time_RT_ns += commands[i].real_time_ns;
            total_search_time_UM += commands[i].time_in_user_mode;
            total_search_time_KM += commands[i].time_in_kernel_mode;

            printf("Operation: remove_index, Arg: %lu\n", remove_cmd[cmd_rm_index]);

            cmd_rm_index++;
        }

        printf("\t Real time executed operation: %ld s\n", commands[i].real_time_s);
        printf("\t Real time executed operation: %ld ns\n", commands[i].real_time_ns);
        printf("\t Time in user mode executed operation: %f s\n", commands[i].time_in_user_mode);
        printf("\t Time in kernel mode executed operation: %f s\n", commands[i].time_in_kernel_mode);
    }

    printf("\n");
    printf("Total time for search_directory:\n");
    printf("\t Real time: %ld s\n", total_search_time_RT_s);
    printf("\t Real time: %ld ns\n", total_search_time_RT_ns);
    printf("\t Time in user mode: %f s\n", total_search_time_UM);
    printf("\t Time in kernel mode: %f s\n", total_search_time_KM);
    printf("\n");
    printf("Total time for remove_index:\n");
    printf("\t Real time: %ld s\n", total_remove_time_RT_s);
    printf("\t Real time: %ld ns\n", total_remove_time_RT_ns);
    printf("\t Time in user mode: %f s\n", total_remove_time_UM);
    printf("\t Time in kernel mode: %f s\n", total_remove_time_KM);

    free(commands);
    free(remove_cmd);
    free(search_cmd);
    clear();

    return 0;
}