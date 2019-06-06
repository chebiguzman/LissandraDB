all: filesystem clean

filesystem: filesystem.o server.o pharser.o console.o
	gcc -o filesystem filesystem.o server.o pharser.o console.o -lcommons -pthread

filesystem.o: filesystem.c
	gcc -c filesystem.c -o filesystem.o -lcommons
	
server.o:
	gcc -c ../server.c -lcommons

pharser.o:
	gcc -c ../pharser.c

console.o:
	gcc -c ../console.c

clean: 
	find . -name "*.o" -type f -delete