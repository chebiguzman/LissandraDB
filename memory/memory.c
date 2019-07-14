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
#include "../pharser.h"
#include "../actions.h"
#include "../console.h"

//logger global para que lo accedan los threads
t_log* logger;
int fs_socket; 
//punto de entrada para el programa y el kernel
int main(int argc, char const *argv[])
{
  //set up config  
  t_config* config = config_create("config");
  char* LOGPATH = config_get_string_value(config, "LOG_PATH");
  int PORT = config_get_int_value(config, "PORT");

  //set up log
  logger = log_create(LOGPATH, "Memory", 1, LOG_LEVEL_INFO);
  log_info(logger, "El log fue creado con exito");

  //set up server
  pthread_t tid;
  server_info* serverInfo = malloc(sizeof(server_info));
  memset(serverInfo, 0, sizeof(server_info));    
  serverInfo->logger = logger;
  serverInfo->portNumber = PORT; 
  int reslt = pthread_create(&tid, NULL, create_server, (void*) serverInfo);

  //set up client 
  fs_socket = socket(AF_INET, SOCK_STREAM, 0);
  char* FS_IP = config_get_string_value(config, "FS_IP");
  int FS_PORT = config_get_int_value(config, "FS_PORT");
  printf("%s %d", FS_IP, FS_PORT);
  struct sockaddr_in sock_client;
  
  sock_client.sin_family = AF_INET; 
  sock_client.sin_addr.s_addr = inet_addr(FS_IP); 
  sock_client.sin_port = htons(FS_PORT);

  int connection_result =  connect(fs_socket, (struct sockaddr*)&sock_client, sizeof(sock_client));
  
  if(connection_result < 0){
    log_error(logger, "No se logro establecer la conexion con el File System");   
  }
  else{
    
  }

  //inicio lectura por consola
  pthread_t tid_console;
  pthread_create(&tid_console, NULL, console_input, "Memory");
    
  
  //Espera a que terminen las threads antes de seguir
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
char* action_select(package_select* select_info){
  log_info(logger, "Se recibio una accion select");
  //char* response = "Sarasa 2"; //TODO serializar el response
  char* response = parse_package_select(select_info);
  char* sarasa = "respuesta muy inteligente de select\n";
  
  return sarasa; //tienen que devolver algo si no se rompe
}

char* action_insert(package_insert* insert_info){
  log_info(logger, "Se recibio una accion insert");
}

//en esta funcion se devuelve lo 
//proveniente del gossiping
//devuelve solo las seeds
//con esta forma: RETARDO_GOSSIPING_DE_ESTA_MEMORIA|id,ip,port|id,ip,port|id,ip,port
//                                                    seed        seed      seed
char* action_intern__status(){
  char* res = strdup("300000000|"); //ya que se puede modificar en tiempo real y yo necesito saber cada cuanto ir a buscar una memoira se le añade como primer elemento el retargo gossiping de la memoria principal.
  char sep[2] = { ',', '\0' };
  char div[2] = { '|', '\0' };
  t_config* config = config_create("config");
  char** ips = config_get_array_value(config, "IP_SEEDS");
  char** ports = config_get_array_value(config, "PUERTO_SEEDS");

  int i = 2; //ESto reprecenta el id de las memorias, se consigue como parte del proceso de gossiping.
  while(*ips != NULL){
    strcat(res, string_itoa(i));
    strcat(res,sep);    
    strcat(res, *ips);
    strcat(res,sep);
    strcat(res, *ports);
    strcat(res,div);
    ips++;
    ports++;
    i++;
  }
  return res;
}

char* action_create(package_create* create_info){
  log_info(logger, "Se recibio una accion create");
}

char* action_describe(package_describe* describe_info){
  log_info(logger, "Se recibio una accion describe");
}

char* action_drop(package_drop* drop_info){
  log_info(logger, "Se recibio una accion drop");
}

char* action_journal(package_journal* journal_info){
  log_info(logger, "Se recibio una accion select");
}

char* action_add(package_add* add_info){
  log_info(logger, "Se recibio una accion select");
}

char* action_run(package_run* run_info){
  log_info(logger, "Se recibio una accion run");
}

char* action_metrics(package_metrics* metrics_info){
  log_info(logger, "Se recibio una accion metrics");
}


char* parse_input(char* input){
  return exec_instr(input);
}
