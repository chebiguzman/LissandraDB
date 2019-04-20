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


void* create_server(void* args){

    server_info* serverInfo = args;
    
	 int sockfd, clilentNumber;
	 struct sockaddr_in serverAddress, cli_addr;
     char buffer[512] = {0};

    
	int portNumber = htons(serverInfo->portNumber);
    
	//esta esturctura esta en in.h y tiene la informacion necesaria para hacer bind() al sv 
	serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = portNumber;
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    //argv0 que la coneccion es por medio de sockets, arv1 tipo TCP , argv2 y devuelve un fd (file decriptor)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    
    if(bind(sockfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress))){

        log_error(serverInfo->logger, "el servidor se cayo por un error en bind()");
        exit(1);
        
    } 

    //escucha al socket con un queue de 5 coneccioens
    listen(sockfd, 5);

    clilentNumber = sizeof(cli_addr);
    log_info(serverInfo->logger, "El servidor se inicializo exitosamente");
    while (1)
    {
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilentNumber);
        log_info(serverInfo->logger, "Servidor esperando mensajes");

        if(newsockfd<0){
            log_error(serverInfo->logger, "el servidor se cayo por un error en accept()");
            exit(1);
        }

        

        read(newsockfd, buffer, sizeof(buffer)); //BUFFER DINAMMICO ALOCAR LO NECESARIO PARA EL MENSAJE 
        log_info(serverInfo->logger, buffer );

    }
    
    
	return 0;
}
