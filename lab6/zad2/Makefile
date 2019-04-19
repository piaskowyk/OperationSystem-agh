compile_command = gcc -Wall

all: server client

server: server.c
	$(compile_command) server.c -o ./server

test_server: server.c
	gcc ./server.c -Wall -g -O0 -o ./test_server

client: client.c
	$(compile_command) client.c -o ./client

test_client: client.c
	gcc ./client.c -Wall -g -O0 -o ./test_client

clean:
	rm -rf *.o server test_server client test_client