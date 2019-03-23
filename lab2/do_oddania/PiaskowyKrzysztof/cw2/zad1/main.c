#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/times.h>
#include <time.h>
#include <zconf.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

struct Generate_cmd{
    char* file_name;
    unsigned int how_records;
    unsigned int block_size;
};

struct Sort_cmd{
    char* file_name;
    unsigned int how_records;
    unsigned int block_size;
    char* type;
};

struct Copy_cmd{
    char* source_file_name;
    char* destination_file_name;
    unsigned int how_records;
    unsigned int block_size;
    char* type;
};

struct Operation{
    float time_in_kernel_mode;
    float time_in_user_mode;
    long real_time_s;
    long real_time_ns;
};

void generate(struct Generate_cmd generate_cmd);
void generate_alphabet(struct Generate_cmd generate_cmd);
int sort(struct Sort_cmd sort_cmd);
int copy(struct Copy_cmd copy_cmd);

void print_time_info();

struct Time{
    struct timespec start_operation, stop_operation;
    struct tms start_operation_cpu, stop_operation_cpu;
    long clock_tick_speed;
};

void timer_init();
void timer_start();
void timer_stop();
void calculateOperationTime();

void testSort(char* file_name, unsigned int block_size);

struct Time time_snapshot;
struct Operation operation;

int main(int argc, char *argv[]) {
    timer_init();
    int function_result = 0;
    int valid_input = 1;
    int show_time_info = 1;

    if(argc <= 1){
        fputs("Not enough input arguments.\n", stderr);
        exit(100);
    }

    //if calling function generate
    if(strncmp(argv[1], "generate", 8) == 0 && argc == 5){
        struct Generate_cmd generate_cmd;
        generate_cmd.file_name = argv[2];
        char *end;
        generate_cmd.how_records = (unsigned int)strtol(argv[3], &end, 0);
        generate_cmd.block_size = (unsigned int)strtol(argv[4], &end, 0);

        if(generate_cmd.how_records <= 0 || generate_cmd.how_records > 9999999){
            valid_input = 0;
            fputs("Number of records must be between [1-9999999].\n", stderr);
        }
        if(generate_cmd.block_size <= 0 || generate_cmd.block_size > 9999999){
            valid_input = 0;
            fputs("Block size can not be smaller than [1-9999999].\n", stderr);
        }

        if(valid_input){
            generate_alphabet(generate_cmd);
//            generate(generate_cmd);
        }
        show_time_info = 0;
        function_result = 1;
    }
        //if calling function sort
    else if(strncmp(argv[1], "sort", 4) == 0 && argc == 6){
        struct Sort_cmd sort_cmd;
        sort_cmd.file_name = argv[2];
        char *end;
        sort_cmd.how_records = (unsigned int)strtol(argv[3], &end, 0);
        sort_cmd.block_size = (unsigned int)strtol(argv[4], &end, 0);
        sort_cmd.type = argv[5];

        if(sort_cmd.how_records <= 0 || sort_cmd.how_records > 9999999){
            valid_input = 0;
            fputs("Number of records must be between [1-9999999].\n", stderr);
        }
        if(sort_cmd.block_size <= 0 || sort_cmd.block_size > 9999999){
            valid_input = 0;
            fputs("Block size can not be smaller than [1-9999999].\n", stderr);
        }

        if(valid_input){
            timer_start();
            function_result = sort(sort_cmd);
            timer_stop();
            calculateOperationTime();
        }

    }
        //if calling function copy
    else if(strncmp(argv[1], "copy", 4) == 0 && argc == 7){
        struct Copy_cmd copy_cmd;
        copy_cmd.source_file_name = argv[2];
        copy_cmd.destination_file_name = argv[3];
        char *end;
        copy_cmd.how_records = (unsigned int)strtol(argv[4], &end, 0);
        copy_cmd.block_size = (unsigned int)strtol(argv[5], &end, 0);
        copy_cmd.type = argv[6];

        if(copy_cmd.how_records <= 0 || copy_cmd.how_records > 9999999){
            valid_input = 0;
            fputs("Number of records must be between [1-9999999].\n", stderr);
        }
        if(copy_cmd.block_size <= 0 || copy_cmd.block_size > 9999999){
            valid_input = 0;
            fputs("Block size can not be smaller than [1-9999999].\n", stderr);
        }

        if(valid_input){
            timer_start();
            function_result = copy(copy_cmd);
            timer_stop();
            calculateOperationTime();
        }
    }
    else {
        valid_input = 0;
        fputs("Invalid input arguments.\n", stderr);
    }

    if(!valid_input){
        exit(101);
    }

    if(!function_result){
        fputs("Error while running function.\n", stderr);
        exit(101);
    }

    if(show_time_info){
        print_time_info();
    }

    return 0;
}

//##########################################################################################

void generate(struct Generate_cmd generate_cmd){
    char* cmd_command = calloc(generate_cmd.how_records, generate_cmd.block_size);
    sprintf(cmd_command,
            "dd if=/dev/urandom iflag=fullblock of=%s count=%i bs=%i",
            generate_cmd.file_name,
            generate_cmd.how_records,
            generate_cmd.block_size
    );
    system(cmd_command);
    free(cmd_command);
}

void generate_alphabet(struct Generate_cmd generate_cmd){
    char* cmd_command = calloc(1, generate_cmd.block_size);
    int n = generate_cmd.block_size;
    int file_writer = open(generate_cmd.file_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    for(int k = 0; k < generate_cmd.how_records; k++) {
        for(int i = 0; i < n; i++){
            char a = (char)((rand()%25)+97);
            cmd_command[i] = a;
        }
        write(file_writer, cmd_command, generate_cmd.block_size);
    }
    free(cmd_command);
}

int sort(struct Sort_cmd sort_cmd) {

    if(strcmp(sort_cmd.type, "sys") == 0) {
        int file_reader, file_writer;
        ssize_t counter;

        file_reader = open(sort_cmd.file_name, O_RDONLY);
        file_writer = open(sort_cmd.file_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

        if(!file_reader || !file_writer){
            printf("Error while opening file.\n");
            return 0;
        }

        char* buffer = calloc(1, sort_cmd.block_size);
        char* min_block = calloc(1, sort_cmd.block_size);
        char* current_block = calloc(1, sort_cmd.block_size);

        for(int i = 0; i < sort_cmd.how_records; i++){
            lseek(file_reader, i * sort_cmd.block_size, SEEK_SET);
            int min = 999999999;
            int index_of_min_block = 0;
            int index = 0;

            while ((counter = read(file_reader, buffer, sort_cmd.block_size)) > 0){
                if(min > buffer[0]){
                    min = buffer[0];
                    index_of_min_block = index;
                }
                index++;
            }

            if(index_of_min_block != 0){
                lseek(file_reader, (i + index_of_min_block) * sort_cmd.block_size, SEEK_SET);
                read(file_reader, min_block, sort_cmd.block_size);

                lseek(file_reader, i * sort_cmd.block_size, SEEK_SET);
                read(file_reader, current_block, sort_cmd.block_size);

                lseek(file_writer, i * sort_cmd.block_size, SEEK_SET);
                write(file_writer, min_block, sort_cmd.block_size);

                lseek(file_writer, (i + index_of_min_block) * sort_cmd.block_size, SEEK_SET);
                write(file_writer, current_block, sort_cmd.block_size);
            }
        }

        close(file_writer);
        close(file_reader);
        free(buffer);
        free(min_block);
        free(current_block);
    }
    else {

        char* buffer = calloc(1, sort_cmd.block_size);
        char* min_block = calloc(1, sort_cmd.block_size);
        char* current_block = calloc(1, sort_cmd.block_size);

        for(int i = 0; i < sort_cmd.how_records; i++){
            FILE* file_reader = fopen(sort_cmd.file_name, "r");
            FILE* file_writer = fopen(sort_cmd.file_name, "r+");
            ssize_t counter;

            if(!file_reader || !file_writer){
                printf("Error while opening file.\n");
                return 0;
            }

            fseek(file_reader, i * sort_cmd.block_size, SEEK_SET);
            int min = 999999;
            int index_of_min_block = 0;
            int index = 0;

            while ((counter = fread(buffer, 1, sort_cmd.block_size, file_reader)) > 0){
                if(min > buffer[0]){
                    min = buffer[0];
                    index_of_min_block = index;
                }
                index++;
            }

            if(index_of_min_block != 0) {
                fseek(file_reader, (i + index_of_min_block) * sort_cmd.block_size, SEEK_SET);
                fread(min_block, 1, sort_cmd.block_size, file_reader);

                fseek(file_reader, i * sort_cmd.block_size, SEEK_SET);
                fread(current_block, 1, sort_cmd.block_size, file_reader);

                fseek(file_writer, i * sort_cmd.block_size, SEEK_SET);
                fwrite(min_block, 1, sort_cmd.block_size, file_writer);
//                fflush(file_writer);

                fseek(file_writer, (i + index_of_min_block) * sort_cmd.block_size, SEEK_SET);
                fwrite(current_block, 1, sort_cmd.block_size, file_writer);
                fflush(file_writer);
            }

            fclose(file_writer);
            fclose(file_reader);
        }


        free(buffer);
        free(min_block);
        free(current_block);
    }
//    testSort(sort_cmd.file_name, sort_cmd.block_size);
    return 1;
}

int copy(struct Copy_cmd copy_cmd){

    if(strcmp(copy_cmd.type, "sys") == 0){
        char* buffer = calloc(1, copy_cmd.block_size);
        int source, destination;
        ssize_t counter;

        source = open(copy_cmd.source_file_name, O_RDONLY);
        destination = open(copy_cmd.destination_file_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

        if(!source || !destination){
            printf("Error while opening file.\n");
            free(buffer);
            return 0;
        }

        while ((counter = read(source, buffer, copy_cmd.block_size)) > 0){
            write(destination, buffer, copy_cmd.block_size);
        }
        free(buffer);
    }
    else{
        char* buffer = calloc(1, copy_cmd.block_size);
        FILE* source = fopen(copy_cmd.source_file_name, "r");
        FILE* destination = fopen(copy_cmd.destination_file_name, "w");

        if(!source || !destination){
            printf("Error while opening file.\n");
            free(buffer);
            fclose(destination);
            fclose(source);
            return 0;
        }

        for(int i = 0; i < copy_cmd.how_records; i++) {
            fread(buffer, 1, copy_cmd.block_size, source);
            fwrite(buffer, 1, copy_cmd.block_size, destination);
        }

        fclose(destination);
        fclose(source);
        free(buffer);
    }

    return 1;
}

void print_time_info(){
    printf("Real time: %ld s : %ld ns, User time %f s, Kernel time: %f s\n",
           operation.real_time_s,
           operation.real_time_ns,
           operation.time_in_user_mode,
           operation.time_in_kernel_mode
    );
}

void timer_init() {
    time_snapshot.clock_tick_speed = sysconf(_SC_CLK_TCK);
}

void timer_start() {
    clock_gettime(CLOCK_REALTIME, &time_snapshot.start_operation);
    times(&time_snapshot.start_operation_cpu);
}

void timer_stop() {
    clock_gettime(CLOCK_REALTIME, &time_snapshot.stop_operation);
    times(&time_snapshot.stop_operation_cpu);
}

void calculateOperationTime() {
    if((time_snapshot.stop_operation.tv_nsec - time_snapshot.start_operation.tv_nsec) < 0){
        operation.real_time_s = time_snapshot.stop_operation.tv_sec - time_snapshot.start_operation.tv_sec - 1;
        operation.real_time_ns = 1000000000 + time_snapshot.stop_operation.tv_nsec - time_snapshot.start_operation.tv_nsec;
    }
    else {
        operation.real_time_s = time_snapshot.stop_operation.tv_sec - time_snapshot.start_operation.tv_sec;
        operation.real_time_ns = time_snapshot.stop_operation.tv_nsec - time_snapshot.start_operation.tv_nsec;
    }
    operation.time_in_kernel_mode = (float)(
            (time_snapshot.stop_operation_cpu.tms_stime - time_snapshot.start_operation_cpu.tms_stime) +
            (time_snapshot.stop_operation_cpu.tms_cstime - time_snapshot.start_operation_cpu.tms_cstime)
    )/time_snapshot.clock_tick_speed;
    operation.time_in_user_mode = (float)(
            (time_snapshot.stop_operation_cpu.tms_utime - time_snapshot.start_operation_cpu.tms_utime) +
            (time_snapshot.stop_operation_cpu.tms_cutime - time_snapshot.start_operation_cpu.tms_cutime)
    )/time_snapshot.clock_tick_speed;
}

void testSort(char* file_name, unsigned int block_size) {
    char* buffer = calloc(1, block_size);
    int file_reader;
    ssize_t counter;
    char last_value = 0;
    file_reader = open(file_name, O_RDONLY);
    while ((counter = read(file_reader, buffer, block_size)) > 0){
        if(last_value > buffer[0]){
            printf("ERROR - %d - %d\n", buffer[0], last_value);
        }
        last_value = buffer[0];
    }
    close(file_reader);
    free(buffer);
}