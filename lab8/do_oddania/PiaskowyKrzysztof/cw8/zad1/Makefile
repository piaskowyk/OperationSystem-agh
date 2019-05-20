compile_command = gcc -Wall

all: main

main: main.c
	$(compile_command) -c -Wall utils.c -o utils.o -lm
	$(compile_command) -c main.c -o main.o -lpthread -lm
	$(compile_command) main.o utils.o -o ./main -lpthread -lm

clean:
	rm -rf *.o main result*