#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/log.h>
#include <commons/string.h>

/*
Esta funcion levanta un servidor por sockets
portNumber: numero del puerto para el servidor
ip: ip del servidor
logger: logueador para poder imprimir status de la conexcion
*/

void* serverThread (int portNumber, char * ip, t_log* logger)
{
	 int sockfd, newsockfd, clilentNumber;
	 struct sockaddr_in serverAddress, cli_addr;


    
	portNumber = htons(portNumber);

	//esta esturctura esta en in.h y tiene la informacion necesaria para hacer bind() al sv 
	serverAddress.sin_family = AF_INET;
    char* log1 = "se setea el servidor a ip: ";
    string_append(&log1, ip);
    log_info(logger, log1);
    serverAddress.sin_addr.s_addr =  inet_addr(ip);
    serverAddress.sin_port = portNumber;

    //argv0 que la coneccion es por medio de sockets, arv1 tipo TCP , argv2 y devuelve un fd (file decriptor)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int bindStatus = bind(sockfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    if(bindStatus<0) exit(1); //TODO logear

    //escucha al socket con un queue de 5 coneccioens
    listen(sockfd, 5);

    
    clilentNumber = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilentNumber);
    if(newsockfd<0) exit(1);
	return 0;
}
