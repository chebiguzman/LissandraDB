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
    
   t_config* config = config_create("config");
    char* LOGPATH = config_get_string_value(config, "LOG_PATH");
    int PORT = config_get_int_value(config, "PORT");

  

   
    //set up log
    t_log* logger;
    pthread_t tid;
    logger = log_create(LOGPATH, "Kernel", 1, LOG_LEVEL_INFO);


    //set up server
    server_info* serverInfo = malloc(sizeof(server_info));
    memset(serverInfo, 0, sizeof( server_info));    
    serverInfo->logger = logger;
    serverInfo->portNumber = PORT;
      
    int reslt = pthread_create(&tid, NULL, create_server, (void*) serverInfo);
    
    //set up client 
    /*int clientfd = socket(AF_INET, SOCK_STREAM, 0); 

    struct sockaddr_in sock_client;
   
    sock_client.sin_family = AF_INET; 
    sock_client.sin_addr.s_addr = inet_addr(MEMORY_IP); 
    sock_client.sin_port = htons(INADDR_ANY);

    int connectS =  connect(clientfd, (struct sockaddr*)&sock_client, sizeof(sock_client));
    printf("coneccion: %d", connectS);
    
    //write(clientfd, "hello world", sizeof("hello world"));
    */

    //JOIN THREADS
    pthread_join(tid,NULL);
    
    //FREE MEMORY
    free(LOGPATH);
    free(logger);
    free(serverInfo);
    config_destroy(config);

      return 0;
}

//IMPLEMENTACION DE FUNCIONES (Devolver errror fuera del subconjunto)
//AUN NO HACERLES IMPLEMENTACION
void action_select(package_select* select_info){
  log_info(logger, "Se recibio una accion select");
}

void action_insert(package_insert* insert_info){
  log_info(logger, "Se recibio una accion insert");
}

void action_create(package_create* create_info){
  log_info(logger, "Se recibio una accion create");
}

void action_describe(package_describe* describe_info){
  log_info(logger, "Se recibio una accion describe");
}

void action_drop(package_drop* drop_info){
  log_info(logger, "Se recibio una accion drop");
}

void action_journal(package_journal* journal_info){
  log_info(logger, "Se recibio una accion select");
}

void action_add(package_add* add_info){
  log_info(logger, "Se recibio una accion select");
}

void action_run(package_run* run_info){
  log_info(logger, "Se recibio una accion run");
}

void action_metrics(package_metrics* metrics_info){
  log_info(logger, "Se recibio una accion metrics");
}
