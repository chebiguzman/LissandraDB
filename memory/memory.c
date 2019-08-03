#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <commons/config.h>
#include "../server.h"
// #include "../pharser.h"
#include <signal.h>

// #include "../actions.h"
#include "../console.h"
#include "segments.h"
#include "gossiping.h"


void exec_err_abort(){};
//logger global para que lo accedan los threads
int main_memory_size;

//punto de entrada para el programa y el kernel
int main(int argc, char const *argv[])
{

  pthread_mutex_init(&gossip_table_mutex, NULL);
  pthread_mutex_lock(&gossip_table_mutex);					
    
  sigset_t set;	
  signal(SIGPIPE, SIG_IGN);
	

  //set up config  
  char* config_name = malloc(10);
  strcpy(config_name, "config");
  strcat(config_name, argv[1]);
  config = config_create(config_name);
  char* LOGPATH = config_get_string_value(config, "LOG_PATH");
  MEMORY_PORT = config_get_int_value(config, "PORT");
  MEMORY_IP = "127.0.0.1";

  //set up log
  logger = log_create(LOGPATH, "Memory", 1, LOG_LEVEL_INFO);
  log_info(logger, "El log fue creado con exito\n");

  //set up server
  pthread_t tid;
  server_info* serverInfo = malloc(sizeof(server_info));
  memset(serverInfo, 0, sizeof(server_info));    
  serverInfo->logger = logger;
  serverInfo->portNumber = MEMORY_PORT; 
  int reslt = pthread_create(&tid, NULL, create_server, (void*) serverInfo);

  //set up fs client 
  fs_socket = socket(AF_INET, SOCK_STREAM, 0);
  char* FS_IP = config_get_string_value(config, "FS_IP");
  int FS_PORT = config_get_int_value(config, "FS_PORT");
  printf("%s %d\n", FS_IP, FS_PORT);
  struct sockaddr_in sock_client;
  
  sock_client.sin_family = AF_INET; 
  sock_client.sin_addr.s_addr = inet_addr(FS_IP); 
  sock_client.sin_port = htons(FS_PORT);

  int connection_result =  connect(fs_socket, (struct sockaddr*)&sock_client, sizeof(sock_client));
  
  if(connection_result < 0){
    log_error(logger, "No se logro establecer la conexion con el File System");
     // return 0;
  }
  else{
    char* handshake = malloc(16);
    write(fs_socket, "MEMORY", strlen("MEMORY"));
    read(fs_socket, handshake, 4);
    VALUE_SIZE = atoi(handshake);
    log_info(logger, "La memory se conecto con fs. El hanshake dio como value size %d", VALUE_SIZE);
  }

  // setup memoria principal
  main_memory_size = config_get_int_value(config, "TAM_MEM");
  MAIN_MEMORY = malloc(main_memory_size);
  if(MAIN_MEMORY == NULL) {
    log_error(logger, "No se pudo alocar espacio para la memoria principal.");
    return 0;
  }
  memset(MAIN_MEMORY, 0, main_memory_size);

  // setup segments
  SEGMENT_TABLE = NULL;
  PAGE_SIZE = sizeof(page_t) - sizeof(char*) + VALUE_SIZE;
  NUMBER_OF_PAGES = main_memory_size / PAGE_SIZE;
  LRU_TABLE = create_LRU_TABLE();

  printf("\n---- Memory info ----\n");
  printf("Main memory size: %d\n", main_memory_size);
  printf("Page size: %d\n", PAGE_SIZE);
  printf("Number of pages: %d\n", NUMBER_OF_PAGES);
  printf("---------------------\n\n");
  
  // setup gossiping
  seeds_ports = config_get_array_value(config, "PUERTO_SEEDS");
  seeds_ips = config_get_array_value(config, "IP_SEEDS");
  GOSSIP_TABLE = NULL;
  gossip_t* this_node = create_node(MEMORY_PORT, MEMORY_IP);
  this_node->number = config_get_int_value(config, "MEMORY_NUMBER");
  add_node(&GOSSIP_TABLE, this_node);
  
  print_gossip_table(&GOSSIP_TABLE);

  pthread_mutex_unlock(&gossip_table_mutex);					

  pthread_mutex_init(&main_memory_mutex, NULL);
  pthread_mutex_init(&segment_table_mutex, NULL);
  pthread_mutex_init(&lru_table_mutex, NULL);

  // inicio gossiping
  pthread_t tid_gossiping;
  pthread_create(&tid_gossiping, NULL, gossip, (void*)&GOSSIP_TABLE);
  
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

char* action_select(package_select* select_info){
 //  log_info(logger, "Se recibio una accion select");
  pthread_mutex_lock(&segment_table_mutex);					
  pthread_mutex_lock(&lru_table_mutex);
  pthread_mutex_lock(&main_memory_mutex);
  page_info_t* page_info = find_page_info(select_info->table_name, select_info->key); // cuando creo paginas en el main y las busco con la misma key, no me las reconoce por alguna razon
  if(page_info != NULL){
    printf("Page found in memory -> Key: %d, Value: %s\n", select_info->key, page_info->page_ptr->value);
    pthread_mutex_unlock(&segment_table_mutex);					
    pthread_mutex_unlock(&lru_table_mutex);
    pthread_mutex_unlock(&main_memory_mutex);

    return page_info->page_ptr->value;
  }
  // si no tengo el segmento, o el segmento no tiene la pagina, se la pido al fs
  printf("Buscando en FileSystem. Tabla: %s, Key:%d...\n", select_info->table_name, select_info->key);  
  char* response = exec_in_fs(fs_socket, parse_package_select(select_info)); 
  printf("Respuesta del FileSystem: %s\n", response);  
  if(strcmp(response, "La tabla solicitada no existe.\n") != 0 && strcmp(response, "Key invalida\n") != 0 && !strcmp(response, "NO SE ENCUENTRA FS")){
    page_t* page = create_page((unsigned)time(NULL), select_info->key, response);
    save_page(select_info->table_name, page);
    printf("Page found in file system. Table: %s, Key: %d, Value: %s\n", select_info->table_name, page->key, page->value);
    pthread_mutex_unlock(&segment_table_mutex);					
    pthread_mutex_unlock(&lru_table_mutex);
    pthread_mutex_unlock(&main_memory_mutex);

    return string_new("%s\n", page->value);
  }
  pthread_mutex_unlock(&segment_table_mutex);					
  pthread_mutex_unlock(&lru_table_mutex);
  pthread_mutex_unlock(&main_memory_mutex);
  return response;
}

char* action_insert(package_insert* insert_info){
  log_info(logger, "Se recibio una accion insert");
 
  pthread_mutex_lock(&segment_table_mutex);					
  pthread_mutex_lock(&lru_table_mutex);
  pthread_mutex_lock(&main_memory_mutex);					
  //BUSCO O CREO EL SEGMENTO
  segment_t*  segment = find_or_create_segment(insert_info->table_name); // si no existe el segmento lo creo.
  page_t* page = create_page(insert_info->timestamp, insert_info->key, insert_info->value);
  page_info_t* page_info = insert_page(insert_info->table_name, page);
  pthread_mutex_unlock(&segment_table_mutex);					
  pthread_mutex_unlock(&lru_table_mutex);
  pthread_mutex_unlock(&main_memory_mutex);
  return strdup("");
}

char* action_create(package_create* create_info){
  log_info(logger, "Se recibio una accion create");
  return exec_in_fs(fs_socket, parse_package_create(create_info)); // retorno el response de fs
}

char* action_describe(package_describe* describe_info){
  log_info(logger, "Se recibio una accion describe");
  return exec_in_fs(fs_socket, parse_package_describe(describe_info)); // retorno el response de fs
}

char* action_drop(package_drop* drop_info){
  log_info(logger, "Se recibio una accion drop");
  pthread_mutex_lock(&segment_table_mutex);					
  pthread_mutex_lock(&lru_table_mutex);
  pthread_mutex_lock(&main_memory_mutex);
  segment_t* segment = find_segment(drop_info->table_name);
  if(segment != NULL) remove_segment(drop_info->table_name, 0);
  pthread_mutex_unlock(&segment_table_mutex);					
  pthread_mutex_unlock(&lru_table_mutex);
  pthread_mutex_unlock(&main_memory_mutex);
  return exec_in_fs(fs_socket, parse_package_drop(drop_info)); // retorno el response de fs
}

char* action_journal(package_journal* journal_info){
  log_info(logger, "Se recibio una accion select");
  pthread_mutex_lock(&segment_table_mutex);					
	pthread_mutex_lock(&lru_table_mutex);
	pthread_mutex_lock(&main_memory_mutex);
  journal();
  pthread_mutex_unlock(&segment_table_mutex);					
	pthread_mutex_unlock(&lru_table_mutex);
	pthread_mutex_unlock(&main_memory_mutex);
  return strdup("Journaling done\n");
}

char* action_add(package_add* add_info){
  return strdup("No es una instruccion valida\n");
}

char* action_run(package_run* run_info){
  return strdup("No es una instruccion valida\n");
}

char* action_metrics(package_metrics* metrics_info){
  return strdup("No es una instruccion valida\n");
}

//en esta funcion se devuelve lo 
//proveniente del gossiping
//devuelve solo las seeds
//con esta forma: ID_PROPIO|RETARDO_GOSSIPING_DE_ESTA_MEMORIA|id,ip,port|id,ip,port|id,ip,port
//                                                    seed        seed      seed
char* action_intern__status(){
  // char* res = strdup("300000000|"); //ya que se puede modificar en tiempo real y yo necesito saber cada cuanto ir a buscar una memoira se le añade como primer elemento el retargo gossiping de la memoria principal.
  pthread_mutex_lock(&gossip_table_mutex);  

  char* retardo_gossip = config_get_string_value(config, "RETARDO_GOSSIPING");
  char* id = config_get_string_value(config, "MEMORY_NUMBER");

  char sep[2] = { ',', '\0' };
  char div[2] = { '|', '\0' };
  printf("algo por aca\n");
  char* gossip_table_buffer = create_gossip_buffer(&GOSSIP_TABLE);
  char* buffer = malloc(strlen(retardo_gossip) + strlen(gossip_table_buffer) + 2);
  *buffer = 0;
  strcpy(buffer, id);
  strcat(buffer, div);
  strcat(buffer, retardo_gossip);
  strcat(buffer, div);
  strcat(buffer, gossip_table_buffer);

  pthread_mutex_unlock(&gossip_table_mutex);  

  log_error(logger, "%s",buffer);
  return buffer;
}

char* parse_input(char* input){
  return exec_instr(input);
}

char* action_gossip(char* arg){
  gossip_t* parsed_gossip_table = parse_gossip_buffer(arg);
  pthread_mutex_lock(&gossip_table_mutex);    					

  printf("Me llego una conexion de una memoria \n");
  char* gossip_buffer = create_gossip_buffer(&GOSSIP_TABLE); // lo creo antes de que compare las tablas asi no le mando las que me acaba de pasar
  printf("- Gossip buffer to send: %s\n", gossip_buffer);

  // tengo que filtrar los nodos. Si me pasan un nodo al cual yo me conecto, no lo tengo que agregar
  // porque si esta desconectado, lo agrega a la tabla igual y no sale nunca porque el que se lo pasa
  // manda su tabla antes de corroborar que este conectado, y despues le pasa lo mismo a ese
  gossip_t* temp_node = parsed_gossip_table;
  while(temp_node != NULL){
      for(int i=0; seeds_ports[i] != NULL; i++){
          int seed_port = atoi(seeds_ports[i]);
          if(temp_node->port ==  seed_port && !strcmp(temp_node->ip, seeds_ips[i])){
              remove_node(&parsed_gossip_table, temp_node);
          }
      }
      temp_node = temp_node->next;
  }
  compare_gossip_tables(&GOSSIP_TABLE, &parsed_gossip_table);

  printf("- Actualizo ");
  print_gossip_table(&GOSSIP_TABLE);

  pthread_mutex_unlock(&gossip_table_mutex);
 
  delete_table(&parsed_gossip_table);
  free(arg);
  return gossip_buffer;
}

