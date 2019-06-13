all: memory clean

memory: memory.o server.o pharser.o console.o segments.o
	gcc -o memory memory.o server.o pharser.o console.o segments.o -lcommons -pthread

memory.o: 
	gcc -c memory.c -lcommons

server.o:
	gcc -c ../server.c -lcommons

pharser.o:
	gcc -c ../pharser.c

console.o:
	gcc -c ../console.c

segments.o:
	gcc -c segments.c
	
clean: 
	find . -name "*.o" -type f -delete