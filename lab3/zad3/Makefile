compile_command = gcc -Wall

all: main

main: main.c
	$(compile_command) -c main.c -o main.o
	$(compile_command) main.o -o ./main

clean_all:
	rm -rf *.o dane* main raport1.txt

clean:
	rm -rf *.o
