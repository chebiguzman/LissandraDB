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

void* serverThread (void* args);

typedef struct {
    int portNumber;
    char* ip;
    t_log* logger;
} server_info;
#endif