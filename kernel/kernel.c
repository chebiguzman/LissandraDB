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
#include "../actions.h"

//punto de entrada para el programa y el kernel

int main(int argc, char const *argv[])
{
    
    //set up confg
    t_config* config = config_create("config");
    char* LOGPATH = config_get_string_value(config, "LOG_PATH");
    int PORT = config_get_int_value(config, "PORT");

    char* MEMORY_IP = config_get_string_value(config, "MEMORY_IP");
    int MEMORY_PORT = config_get_int_value(config, "MEMORY_PORT");

   
    //set up log
    t_log* logger;
    pthread_t tid;
    logger = log_create(LOGPATH, "Kernel", 1, LOG_LEVEL_INFO);


    //set up server
    server_info* serverInfo = malloc(sizeof(server_info));
    memset(serverInfo, 0, sizeof( server_info));    
    serverInfo->logger = logger;
    serverInfo->portNumber = PORT;
    pthread_create(&tid, NULL, create_server, (void*) serverInfo);
    
    //set up client 
    int clientfd = socket(AF_INET, SOCK_STREAM, 0); 
    struct sockaddr_in sock_client;
    sock_client.sin_family = AF_INET; 
    sock_client.sin_addr.s_addr = inet_addr(MEMORY_IP); 
    sock_client.sin_port = htons(MEMORY_PORT);

    int conection_result = connect(clientfd, (struct sockaddr*)&sock_client, sizeof(sock_client));

    if(conection_result<0){
      log_error(logger, "No se logro establecer coneccion con memoria");
      
    }
    write(clientfd, "hello world", sizeof("hello world"));


    

    //JOIN THREADS
    pthread_join(tid,NULL);
    
    //FREE MEMORY
    free(LOGPATH);
    free(logger);
    free(serverInfo);
    config_destroy(config);

      return 0;
}






