all: kernel clean

kernel: kernel.o server.o pharser.o scheduler.o kmemory.o console.o list_helper.o
	gcc -o kernel kernel.o server.o pharser.o scheduler.o kmemory.o list_helper.o console.o -lcommons -pthread

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

kmemory.o:
	gcc -c kmemory.c 

console.o:
	gcc -c ../console.c

list_helper.o:
	gcc -c list_helper.c

clean: 
	find . -name "*.o" -type f -delete