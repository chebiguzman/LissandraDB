all: kernel clean

kernel: kernel.o server.o pharser.o
	gcc -o kernel kernel.o server.o pharser.o -lcommons -pthread

kernel.o: 
	gcc -c kernel.c -lcommons

server.o:
	gcc -c ../server.c -lcommons

pharser.o:
	gcc -c ../pharser.c

clean: 
	find . -name "*.o" -type f -delete