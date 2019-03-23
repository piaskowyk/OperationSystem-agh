compile_command = gcc -Wall

remove_command = rm -rf *.o *.a

arg= \
    create_table 500 \
    search_directory /usr/lib *gimp* tmp_name \
    search_directory /usr/lib Makefile tmp_name \
    search_directory /home/mleko *2* mp_name \
    search_directory /usr/lib *1* tmp_name \
    remove_block 0 \
    remove_block 0 \
    remove_block 0 \
    remove_block 0

test: 
	make clean
	make static
	make shared
	make dynamic
	./run_test.sh

#renerate executablie file with static library but difference optymalization

main_static: ../zad2/main.c
	$(compile_command) -c ../zad1/find_operation.c -o find_operation.o
	ar rcs libfind_operation.a find_operation.o
	$(compile_command) -c ../zad2/main.c -o main.o
	$(compile_command) main.o libfind_operation.a -o ./main_static
	$(remove_command)

static: main_static

#renerate executablie file with shared library but difference optymalization
main_shared: ../zad2/main.c
	$(compile_command) -fPIC -c ../zad1/find_operation.c
	$(compile_command) -shared -Wl,-soname,libfind_operation.so -o libfind_operation.so find_operation.o -lc
	$(compile_command) ../zad2/main.c -lfind_operation -L./ -o main_shared
	$(remove_command)

shared: main_shared

#renerate executablie file with shared library but difference optymalization, load dynamicly
main_dynamic: main_dl.c
	$(compile_command) -fPIC -c ../zad1/find_operation.c
	$(compile_command) -shared -Wl,-rpath,.,-soname,libfind_operation.so -o libfind_operation.so find_operation.o
	$(compile_command) main_dl.c -lfind_operation -L./ -o main_dynamic -ldl
	$(remove_command)

dynamic: main_dynamic

clean:
	rm -rf *.o *.so *.a main_s* main_dy* tmp* mp*

clean_all:
	rm -rf *.o *.so *.a main_s* main_dy* tmp* mp* res*
