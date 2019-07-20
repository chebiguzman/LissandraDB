all: memory clean

memory: memory.o server.o pharser.o console.o segments.o
	gcc -o memory memory.o server.o pharser.o console.o segments.o -lcommons -pthread

memory.o: 
	gcc -c memory.c -lcommons

server.o:
	gcc -c ../server.c -lcommons

pharser.o:
	gcc -g  -c ../pharser.c

console.o:
	gcc -g  -c ../console.c

segments.o:
	gcc -g  -c segments.c
	
clean: 
	find . -name "*.o" -type f -delete