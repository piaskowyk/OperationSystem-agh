compile_command = gcc -Wall

all: sender catcher

sender: sender.c
	$(compile_command) sender.c -o ./sender

catcher: catcher.c
	$(compile_command) catcher.c -o ./catcher

clean:
	rm -rf sender catcher
