all: kernel clean

kernel: kernel.o server.o 
	gcc -o kernel.kernel kernel.o server.o -lcommons

kernel.o: 
	gcc -c kernel.c -lcommons

server.o:
	gcc -c ../server.c -lcommons

clean: 
	find . -name "*.o" -type f -delete