all: kernel clean

kernel: kernel.o server.o 
	gcc -o memory.m memory.o server.o -lcommons -pthread

kernel.o: 
	gcc -c memory.c -lcommons

server.o:
	gcc -c ../server.c -lcommons

clean: 
	find . -name "*.o" -type f -delete