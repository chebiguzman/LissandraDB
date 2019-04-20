#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
#include "../server.h"
#include <commons/config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

//punto de entrada para el programa y el kernel
int main(int argc, char const *argv[])
{
    
    //set up confg
    t_config* config = config_create("config");
    char* LOGPATH = config_get_string_value(config, "LOG_PATH");
    char* IP = config_get_string_value(config, "IP");
    int PORT = config_get_int_value(config, "PORT");

   
    //set up log
    t_log* logger;
    pthread_t tid;
    logger = log_create(LOGPATH, "Memory", 1, LOG_LEVEL_INFO);
    //set up server
    server_info* serverInfo = malloc(sizeof(server_info));
    memset(serverInfo, 0, sizeof( server_info));    
    serverInfo->logger = logger;
    serverInfo->portNumber = PORT;
    
    strncpy(serverInfo->ip, IP , sizeof(serverInfo->ip));
    
    int reslt = pthread_create(&tid, NULL, create_server, (void*) serverInfo);
    
    //set up client 
    /*int clientfd = socket(AF_INET, SOCK_STREAM, 0); 

    struct sockaddr_in sock_client;
   
    sock_client.sin_family = AF_INET; 
    sock_client.sin_addr.s_addr = inet_addr(IP); 
    sock_client.sin_port = htons(PORT);

    int connectS =  connect(clientfd, (struct sockaddr*)&sock_client, sizeof(sock_client));
    printf("coneccion: %d", connectS);
    //write(clientfd, "hello world", sizeof("hello world"));
    */

    //JOIN THREADS
    pthread_join(tid,NULL);
    
    //FREE MEMORY
    free(LOGPATH);
    free(config);
    free(IP);
    free(logger);
    free(serverInfo);

      return 0;
}


