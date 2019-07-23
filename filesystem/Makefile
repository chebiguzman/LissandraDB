all: filesystem clean

filesystem: server.o pharser.o console.o memtable.o engine.o filesystem.o
	gcc -g -o filesystem filesystem.o server.o pharser.o console.o engine.o memtable.o -lcommons -pthread

filesystem.o: filesystem.c
	gcc -g -c filesystem.c -o filesystem.o -lcommons
	
server.o:
	gcc  -g -c ../server.c -lcommons

pharser.o:
	gcc -g  -c ../pharser.c

console.o:
	gcc -g  -c ../console.c

engine.o:
	gcc -g  -c engine.c

memtable.o:
	gcc -g  -c memtable.c

clean: 
	find . -name "*.o" -type f -delete