all: kernel clean

kernel: test.o server.o pharser.o  console.o
	gcc -g -o test test.o server.o pharser.o console.o -lcommons -pthread

test.o: 
	gcc -g -c test.c -lcommons

server.o:
	gcc -g -c ../server.c -lcommons

pharser.o:
	gcc -g -c ../pharser.c


console.o:
	gcc -g  -c ../console.c

clean: 
	find . -name "*.o" -type f -delete