compile_command = gcc -Wall

all: static

main: main.c
	$(compile_command) -c -Wall ../zad1/find_operation.c -o find_operation.o
	ar rcs libfind_operation.a find_operation.o
	$(compile_command) -c main.c -o main.o
	$(compile_command) main.o libfind_operation.a -o ./main

static: main

clean:
	rm -rf find_operation.o raport2.txt

clean_all:
	rm -rf find_operation.o libfind_operation.so libfind_operation.a libfind_operation.so* main main.o raport2.txt