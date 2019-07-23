all: kernel clean

kernel: kernel.o server.o pharser.o scheduler.o kmemory.o console.o metrics.o
	gcc -g -o kernel kernel.o server.o pharser.o scheduler.o kmemory.o metrics_worker.o console.o -lcommons -pthread

kernel.o: 
	gcc -g -c kernel.c -lcommons

server.o:
	gcc -g -c ../server.c -lcommons

pharser.o:
	gcc -g -c ../pharser.c

scheduler.o:
	gcc -g -c scheduler.c

exec.o:
	gcc -g -c exec.c

kmemory.o:
	gcc -g -c kmemory.c 

metrics.o:
	gcc -g -c metrics_worker.c

console.o:
	gcc -g -c ../console.c

clean: 
	find . -name "*.o" -type f -delete