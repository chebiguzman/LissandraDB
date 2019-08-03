#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> //sock structures
#include <commons/log.h> //loggger
#include <commons/string.h> //string append
#include <unistd.h> //read function
#include "server.h"
#include <commons/error.h>
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include "pharser.h"
#include <pthread.h>
#include <string.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#define BUFFER_SOKET_SIZE 3000

void* create_server(void* args){

    server_info* serverInfo = args;
    
	int sockfd, clilentNumber;
	struct sockaddr_in serverAddress, cli_addr;
    char buffer[3000];

    
	int portNumber = htons(serverInfo->portNumber);
    
	//esta esturctura esta en in.h y tiene la informacion necesaria para hacer bind() al sv 
	serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = portNumber;
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    //argv0 que la coneccion es por medio de sockets, arv1 tipo TCP , argv2 y devuelve un fd (file decriptor)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int activado = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

    if(bind(sockfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress))){

        log_error(serverInfo->logger, "el servidor se cayo por un error en bind()");
        exit(1);
        
    } 

    //escucha al socket con un queue de 5 coneccioens
    listen(sockfd, 5);
    log_info(serverInfo->logger, "El servidor se inicializo exitosamente");
    
    while(1){  

        clilentNumber = sizeof(cli_addr);
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilentNumber);
                
        if(newsockfd<0){

            log_error(serverInfo->logger, "el servidor se cayo por un error en accept()");
            
        }

        log_info(serverInfo->logger, "El servidor se conecto exitosamente");
         while (1){

            if(read(newsockfd, buffer, 3000)){
                //log_info(serverInfo->logger, buffer);
                char* responce = parse_input(buffer);

                write(newsockfd,responce,strlen(responce)+1);

                free(responce);
            
            }else{


                 break;

             }//SI el cliente se desconecta deja de leer para ir a por otro cliente

         }    
    
    }
    
    
    
	return 0;
}

void* create_server_fs(void* args){

    server_info* serverInfo = args;
    
	int sockfd, clilentNumber;
	struct sockaddr_in serverAddress, cli_addr;
    fd_set readfds;   

    
	int portNumber = htons(serverInfo->portNumber);
    
	//esta esturctura esta en in.h y tiene la informacion necesaria para hacer bind() al sv 
	serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = portNumber;
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    //argv0 que la coneccion es por medio de sockets, arv1 tipo TCP , argv2 y devuelve un fd (file decriptor)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int activado = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

    if(bind(sockfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress))){

        log_error(serverInfo->logger, "el servidor se cayo por un error en bind()");
        exit(1);
        
    } 

    //escucha al socket con un queue de 5 coneccioens
    listen(sockfd, 5);
    log_info(serverInfo->logger, "El servidor se inicializo exitosamente");
    
    while(1){  

        clilentNumber = sizeof(cli_addr);
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilentNumber);
        int *clientfd = malloc(sizeof(int));
        *clientfd = newsockfd;
        if(newsockfd<0){

            log_error(serverInfo->logger, "el servidor se cayo por un error en accept()");
            
        }

        pthread_t tid;
        pthread_create(&tid, NULL, comunication, (void*) clientfd);
        log_info(serverInfo->logger, "El servidor se conecto exitosamente");
            
    
    }
    
    
    
	return 0;
}
void* comunication(void* args){
    int* clientfd = (int*) args;
    printf("se conecto un cliente con fd %d\n", *clientfd);
    char buffer[3000];
   
    while (1){

        if(read(*clientfd, buffer, 3000)){
            char* responce = parse_input(buffer);

            write(*clientfd,responce,strlen(responce)+1);

            free(responce);
        
        }else{


                break;

        }
    }

}
