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
    char* PORT = config_get_string_value(config, "PORT");

    
    //set up log
    t_log* logger;
    pthread_t tid;
    logger = log_create(LOGPATH, "Kernel", 1, LOG_LEVEL_INFO);
    //set up server
    server_info* serverInfo = malloc(sizeof(server_info));
    memset(serverInfo, 0, sizeof( server_info));    
    serverInfo->logger = logger;
    serverInfo->portNumber = PORT;
    serverInfo->ip[0] = '\0';
    
    int reslt = pthread_create(&tid, NULL, create_server, (void*) serverInfo);   

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


