#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include "../server.h"
char* LOGPATH = "kernel.log";
//punto de entrada para el programa y el kernel
int main(int argc, char const *argv[])
{
    //TODO levantar archivo de configuracion para saber path del log
    
    t_log* logger;
    logger = log_create(LOGPATH, "Kernel", 1, LOG_LEVEL_INFO);
    pthread_t tid;

    int port = 8080;//TODO levantar de config
    server_info* serverInfo;
    serverInfo->ip = NULL;
    serverInfo->logger = logger;
    serverInfo->portNumber = port;
    int reslt = pthread_create(&tid, NULL, serverThread, (void*) serverInfo);
    printf("%d",reslt);
     

    pthread_join(tid,NULL);
      return 0;
}
