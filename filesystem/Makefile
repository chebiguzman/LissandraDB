all: filesystem clean

filesystem: server.o pharser.o console.o memtable.o engine.o filesystem.o
	gcc -o filesystem filesystem.o server.o pharser.o console.o engine.o memtable.o -lcommons -pthread

filesystem.o: filesystem.c
	gcc -c filesystem.c -o filesystem.o -lcommons
	
server.o:
	gcc -c ../server.c -lcommons

pharser.o:
	gcc -c ../pharser.c

console.o:
	gcc -c ../console.c

engine.o:
	gcc -c engine.c

memtable.o:
	gcc -c memtable.c

clean: 
	find . -name "*.o" -type f -delete