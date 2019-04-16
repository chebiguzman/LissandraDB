kernel: kernel.o server.o
	gcc -o kernel/kernel.kernel kernel.o server.o -lcommons

kernel.o: 
	gcc -c kernel/kernel.c -lcommons

server.o:	server.c
	gcc -c server.c -lcommons

