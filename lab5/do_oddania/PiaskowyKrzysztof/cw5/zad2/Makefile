compile_command = gcc -Wall

all: master slave

master: master.c
	$(compile_command) master.c -o ./master_p

test_master: master.c
	gcc ./master.c -Wall -g -O0 -o ./test_master_p

slave: slave.c
	$(compile_command) slave.c -o ./slave_p

test_slave: slave.c
	gcc ./slave.c -Wall -g -O0 -o ./test_slave_p

clean:
	rm -rf *.o master_p test_master_p slave_p test_slave_p pipe_master