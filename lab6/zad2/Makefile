compile_command = gcc -Wall

all: server client

server: server.c
	$(compile_command) server.c -o server -lrt

client: client.c
	$(compile_command) client.c -o client -lrt

clean:
	rm -rf *.o server test_server client test_client