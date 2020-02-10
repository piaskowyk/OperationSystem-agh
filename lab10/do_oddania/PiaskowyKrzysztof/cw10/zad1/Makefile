compile_command = gcc -Wall

all: server client

server: server.c
	$(compile_command) -c -Wall utils.c -o utils.o
	$(compile_command) -c server.c -o server.o -lpthread
	$(compile_command) server.o utils.o -o ./server -lpthread

client: client.c
	$(compile_command) -c -Wall utils.c -o utils.o
	$(compile_command) -c client.c -o client.o -lpthread
	$(compile_command) client.o utils.o -o ./client -lpthread

clean:
	rm -rf *.o main