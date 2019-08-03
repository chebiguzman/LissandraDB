#ifndef SERVER_H_   
#define SERVER_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> //sock structures
#include <commons/log.h> //loggger
#include <commons/string.h> //string append
#include <unistd.h> //read function
#define INSTRUCTION_BYTE_SIZE 9 //tambien cargar en el pharser

void* create_server_memory (void* args);

typedef enum { EVENTUAL_CONSISTENCY, STRONG_CONSISTENCY, STRONG_HASH_CONSISTENCY } consistency_type ;

//SE ESTAN MOVIENDO LAS ESTRUCTURAS A PHARSER.c









//SERVER
typedef struct {
    int portNumber;
    t_log* logger;
} server_info;
#endif

