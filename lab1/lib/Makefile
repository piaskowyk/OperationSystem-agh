compile_command = gcc -Wall

all: static shared

static:
	$(compile_command) -c -Wall find_operation.c -o find_operation.o
	ar rcs libfind_operation.a find_operation.o
	
shared:
	$(compile_command) -fPIC -c find_operation.c
	$(compile_command) -shared -Wl,-soname,libfind_operation.so.1 -o libfind_operation.so.1.0.0 find_operation.o

clean:
	rm -rf find_operation.o libfind_operation.so libfind_operation.a libfind_operation.so*
