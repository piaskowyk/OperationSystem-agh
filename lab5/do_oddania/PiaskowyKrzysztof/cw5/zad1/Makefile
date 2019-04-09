compile_command = gcc -Wall

all: main

main1: main.c
	$(compile_command) main.c -o ./main

test: main.c
	gcc ./main.c -Wall -g -O0 -o ./test_main

clean:
	rm -rf *.o main test_main