compile_command = gcc -Wall

all: trucker test_loader

trucker: trucker.c
	$(compile_command) trucker.c -o ./trucker

test_trucker: trucker.c
	gcc ./trucker.c -Wall -g -O0 -o ./test_trucker

loader: loader.c
	$(compile_command) loader.c -o ./loader

test_loader: loader.c
	gcc ./loader.c -Wall -g -O0 -o ./test_loader

clean:
	rm -rf *.o trucker test_trucker loader test_loader mem