all: filesystem clean

filesystem: filesystem.o server.o
	gcc -o filesystem filesystem.o server.o -lcommons -pthread

filesystem.o: filesystem.c
	gcc -c filesystem.c -o filesystem.o -lcommons
	
server.o:
	gcc -c ..server.c -lcommons

clean: 
	find . -name "*.o" -type f -delete