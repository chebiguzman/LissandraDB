all: kernel clean

kernel: kernel.o server.o pharser.o scheduler.o console.o 
	gcc -o kernel kernel.o server.o pharser.o scheduler.o console.o -lcommons -pthread

kernel.o: 
	gcc -c kernel.c -lcommons

server.o:
	gcc -c ../server.c -lcommons

pharser.o:
	gcc -c ../pharser.c

scheduler.o:
	gcc -c scheduler.c

exec.o:
	gcc -c exec.c

console.o:
	gcc -c ../console.c

clean: 
	find . -name "*.o" -type f -delete