#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char const *argv[])
{
	 int sockfd, newsockfd, portNumber, clilentNumber;
	 struct sockaddr_in serverAddress, cli_addr;

	 //if(argv < 3)	exit(1); //TODO loguear error

	portNumber = htons(atoi(argv[1]));

	//esta esturctura esta en in.h y tiene la informacion necesaria para hacer bind() al sv 
	serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
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