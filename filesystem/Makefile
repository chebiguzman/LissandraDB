all: kernel clean

kernel: kernel.o server.o 
	gcc -o kernel.k kernel.o server.o -lcommons -pthread

kernel.o: 
	gcc -c kernel.c -lcommons

server.o:
	gcc -c ../server.c -lcommons

clean: 
	find . -name "*.o" -type f -delete