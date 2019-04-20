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

void* create_server (void* args);

typedef struct 
{
    char* header[9];
    char* arg0 [491];
    int arg1;
    int arg2;
    int arg3;

}pakage_post;

typedef struct 
{
    //char* ???[]
} pakage_response;



typedef struct {
    int portNumber;
    char ip[13];
    t_log* logger;
} server_info;
#endif

