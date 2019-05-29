compile_command = gcc -Wall
compile_command_test = gcc -Wall -g -O0

all: main

main: main.c
	$(compile_command) -c -Wall utils.c -o utils.o
	$(compile_command) -c main.c -o main.o -lpthread
	$(compile_command) main.o utils.o -o ./main -lpthread

test: main.c
	$(compile_command_test) -c -Wall utils.c -o utils.o
	$(compile_command_test) -c main.c -o main.o -lpthread
	$(compile_command_test) main.o utils.o -o ./test_main -lpthread

clean:
	rm -rf *.o main