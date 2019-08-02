all: memory clean

memory: memory.o server.o pharser.o console.o segments.o gossiping.o
	gcc -Wall -Werror -g -o memory memory.o server.o pharser.o console.o segments.o gossiping.o -lcommons -pthread

memory.o: 
	gcc -g -c memory.c -lcommons

server.o:
	gcc -g -c ../server.c -lcommons

pharser.o:
	gcc -g  -c ../pharser.c

console.o:
	gcc -g  -c ../console.c

segments.o:
	gcc -g  -c segments.c

gossiping.o:
	gcc -g  -c gossiping.c
	
clean: 
	find . -name "*.o" -type f -delete