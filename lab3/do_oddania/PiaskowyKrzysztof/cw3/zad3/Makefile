compile_command = gcc -Wall

all: main tester

main: main.c
	$(compile_command) main.c -o ./main

tester: tester.c
	$(compile_command) tester.c -o ./tester

clean:
	rm -rf main tester ./archiwum ./tmp lista.txt
