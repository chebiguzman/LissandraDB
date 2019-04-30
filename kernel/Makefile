all: kernel clean

kernel: kernel.o server.o pharser.o console.o
	gcc -o kernel kernel.o server.o pharser.o console.o -lcommons -pthread

kernel.o: 
	gcc -c kernel.c -lcommons

server.o:
	gcc -c ../server.c -lcommons

pharser.o:
	gcc -c ../pharser.c

console.o:
	gcc -c ../console.c

clean: 
	find . -name "*.o" -type f -delete