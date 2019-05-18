all: filesystem clean

filesystem: filesystem.o server.o pharser.o console.o fs_structure.o
	gcc -o filesystem filesystem.o server.o pharser.o console.o fs_structure.o -lcommons -pthread

filesystem.o: filesystem.c
	gcc -c filesystem.c -o filesystem.o -lcommons
	
server.o:
	gcc -c ../server.c -lcommons

pharser.o:
	gcc -c ../pharser.c

console.o:
	gcc -c ../console.c

fs_structure.o:
	gcc -c fs_structure.c

clean: 
	find . -name "*.o" -type f -delete