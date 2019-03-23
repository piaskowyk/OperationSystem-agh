compile_command = gcc -Wall

main1: main1.c
	$(compile_command) -c main1.c -o main1.o
	$(compile_command) main1.o -o ./main1

main2: main2.c
	$(compile_command) -c main2.c -o main2.o
	$(compile_command) main2.o -o ./main2

clean_all:
	rm -rf *.o main1 main2

clean:
	rm -rf *.o
