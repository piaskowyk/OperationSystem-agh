compile_command = gcc -Wall

all: main

main: main.c
	$(compile_command) -c -Wall utils.c -o utils.o
	$(compile_command) -c main.c -o main.o -lpthread
	$(compile_command) main.o utils.o -o ./main -lpthread

clean:
	rm -rf *.o main result*