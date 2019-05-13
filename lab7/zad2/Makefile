compile_command = gcc -Wall

all: trucker test_loader

trucker: trucker.c
	$(compile_command) trucker.c -o ./trucker -std=gnu99 -lpthread -lrt

test_trucker: trucker.c
	gcc ./trucker.c -Wall -g -O0 -o ./test_trucker -std=gnu99 -lpthread -lrt

loader: loader.c
	$(compile_command) loader.c -o ./loader -std=gnu99 -lpthread -lrt

test_loader: loader.c
	gcc ./loader.c -Wall -g -O0 -o ./test_loader -std=gnu99 -lpthread -lrt

clean:
	rm -rf *.o trucker test_trucker loader test_loader mem