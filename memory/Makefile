all: memory clean

memory: memory.o server.o pharser.o
	gcc -o memory memory.o server.o pharser.o -lcommons -pthread

memory.o: 
	gcc -c memory.c -lcommons

server.o:
	gcc -c ../server.c -lcommons

pharser.o:
	gcc -c ../pharser.c

clean: 
	find . -name "*.o" -type f -delete