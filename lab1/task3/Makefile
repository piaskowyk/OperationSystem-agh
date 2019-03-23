compile_command_o0 = gcc -Wall -O0
compile_command_o1 = gcc -Wall -O1
compile_command_o2 = gcc -Wall -O2
compile_command_o3 = gcc -Wall -O3
compile_command_os = gcc -Wall -Os

remove_command = rm -rf *.o *.a

#renerate executablie file with static library but difference optymalization

main_static_o0: ../main.c
	$(compile_command_o0) -c ../lib/find_operation.c -o find_operation.o
	ar rcs libfind_operation.a find_operation.o
	$(compile_command_o0) -c ../main.c -o main.o
	$(compile_command_o0) main.o libfind_operation.a -o ./main_static_o0
	$(remove_command)

static_o0 : main_static_o0

main_static_o1: ../main.c
	$(compile_command_o1) -c ../lib/find_operation.c -o find_operation.o
	ar rcs libfind_operation.a find_operation.o
	$(compile_command_o1) -c ../main.c -o main.o
	$(compile_command_o1) main.o libfind_operation.a -o ./main_static_o1
	$(remove_command)

static_o1 : main_static_o1

main_static_o2: ../main.c
	$(compile_command_o2) -c ../lib/find_operation.c -o find_operation.o
	ar rcs libfind_operation.a find_operation.o
	$(compile_command_o2) -c ../main.c -o main.o
	$(compile_command_o2) main.o libfind_operation.a -o ./main_static_o2
	$(remove_command)

static_o2 : main_static_o2

main_static_o3: ../main.c
	$(compile_command_o3) -c ../lib/find_operation.c -o find_operation.o
	ar rcs libfind_operation.a find_operation.o
	$(compile_command_o3) -c ../main.c -o main.o
	$(compile_command_o3) main.o libfind_operation.a -o ./main_static_o3
	$(remove_command)

static_o3 : main_static_o3

main_static_os : ../main.c
	$(compile_command_os) -c ../lib/find_operation.c -o find_operation.o
	ar rcs libfind_operation.a find_operation.o
	$(compile_command_os) -c ../main.c -o main.o
	$(compile_command_os) main.o libfind_operation.a -o ./main_static_os
	$(remove_command)

static_os : main_static_os

#renerate executablie file with shared library but difference optymalization
main_shared_o0: ../main.c
	$(compile_command_o0) -fPIC -c ../lib/find_operation.c
	$(compile_command_o0) -shared -Wl,-soname,libfind_operation.so -o libfind_operation.so find_operation.o -lc
	$(compile_command_o0) ../main.c -lfind_operation -L./ -o main_shared_o0
	$(remove_command)

shared_o0: main_shared_o0

main_shared_o1: ../main.c
	$(compile_command_o0) -fPIC -c ../lib/find_operation.c
	$(compile_command_o0) -shared -Wl,-soname,libfind_operation.so -o libfind_operation.so find_operation.o -lc
	$(compile_command_o0) ../main.c -lfind_operation -L./ -o main_shared_o1
	$(remove_command)

shared_o1: main_shared_o1

main_shared_o2: ../main.c
	$(compile_command_o0) -fPIC -c ../lib/find_operation.c
	$(compile_command_o0) -shared -Wl,-soname,libfind_operation.so -o libfind_operation.so find_operation.o -lc
	$(compile_command_o0) ../main.c -lfind_operation -L./ -o main_shared_o2
	$(remove_command)

shared_o2: main_shared_o2

main_shared_o3: ../main.c
	$(compile_command_o0) -fPIC -c ../lib/find_operation.c
	$(compile_command_o0) -shared -Wl,-soname,libfind_operation.so -o libfind_operation.so find_operation.o -lc
	$(compile_command_o0) ../main.c -lfind_operation -L./ -o main_shared_o3
	$(remove_command)

shared_o3: main_shared_o3

main_shared_os: ../main.c
	$(compile_command_o0) -fPIC -c ../lib/find_operation.c
	$(compile_command_o0) -shared -Wl,-soname,libfind_operation.so -o libfind_operation.so find_operation.o -lc
	$(compile_command_o0) ../main.c -lfind_operation -L./ -o main_shared_os
	$(remove_command)

shared_os: main_shared_os

#renerate executablie file with shared library but difference optymalization, load dynamicly
main_dynamic_o0: ../main_dl.c
	$(compile_command_o0) -fPIC -c ../lib/find_operation.c
	$(compile_command_o0) -shared -Wl,-soname,libfind_operation.so -o libfind_operation.so find_operation.o
	$(compile_command_o0) ../main_dl.c -lfind_operation -L./ -o main_dynamic_o0 -ldl
	$(remove_command)

dynamic_o0: main_dynamic_o0

main_dynamic_o1: ../main_dl.c
	$(compile_command_o0) -fPIC -c ../lib/find_operation.c
	$(compile_command_o0) -shared -Wl,-soname,libfind_operation.so -o libfind_operation.so find_operation.o
	$(compile_command_o0) ../main_dl.c -lfind_operation -L./ -o main_dynamic_o1 -ldl
	$(remove_command)

dynamic_o1: main_dynamic_o1

main_dynamic_o2: ../main_dl.c
	$(compile_command_o0) -fPIC -c ../lib/find_operation.c
	$(compile_command_o0) -shared -Wl,-soname,libfind_operation.so -o libfind_operation.so find_operation.o
	$(compile_command_o0) ../main_dl.c -lfind_operation -L./ -o main_dynamic_o2 -ldl
	$(remove_command)

dynamic_o2: main_dynamic_o2

main_dynamic_o3: ../main_dl.c
	$(compile_command_o0) -fPIC -c ../lib/find_operation.c
	$(compile_command_o0) -shared -Wl,-soname,libfind_operation.so -o libfind_operation.so find_operation.o
	$(compile_command_o0) ../main_dl.c -lfind_operation -L./ -o main_dynamic_o3 -ldl
	$(remove_command)

dynamic_o3: main_dynamic_o3

main_dynamic_os: ../main_dl.c
	$(compile_command_o0) -fPIC -c ../lib/find_operation.c
	$(compile_command_o0) -shared -Wl,-soname,libfind_operation.so -o libfind_operation.so find_operation.o
	$(compile_command_o0) ../main_dl.c -lfind_operation -L./ -o main_dynamic_os -ldl
	$(remove_command)

dynamic_os: main_dynamic_os

clean:
	rm -rf *.o *.so *.a main*
